#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include "tar.h"
#include "string_traitement.h"
#include "tsh_memory.h"
#include "cd.h"

tsh_memory old_memory;

//create a 512 bytes block of zero byte, in order to compare with header
int is_a_end_bloc(struct posix_header *header){
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    if(memcmp(header, end_bloc, 512) == 0) return 1; //identical
    return 0;
}

//return the number total of block in a tar (i.e its size)
int number_of_block(int fd){
    char buffer[512];
    int ret = 0;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, buffer, 512) > 0)
        ret++;
    return ret;
}

//function that counts the number of content block to delete
//returns -1 if such file/directory is not found
int rm_in_tar_aux(int fd, char* full_path, int *nb_block_to_delete, off_t *debut, int arg_r){
    lseek(fd, 0, SEEK_SET);

    int filesize = 0;
    struct posix_header hd;

    while(read(fd, &hd, BLOCKSIZE) > 0){
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + BLOCKSIZE -1) / BLOCKSIZE;
        if(strncmp(hd.name, full_path, strlen(full_path)) == 0){
            *debut = lseek(fd, 0, SEEK_CUR);
            if(arg_r == 0){
                if(hd.typeflag != '5'){ //if it's not a directory
                    *nb_block_to_delete = nb_bloc_fichier;
                    return 1;
                }
            } else { //argument -r was given as option
                while(strncmp(hd.name, full_path,strlen(full_path)) == 0){
                    sscanf(hd.size, "%o", &filesize);
                    nb_bloc_fichier = (filesize + BLOCKSIZE -1) / BLOCKSIZE;
                    *nb_block_to_delete += 1 + nb_bloc_fichier;

                    lseek(fd, nb_bloc_fichier*512, SEEK_CUR);//jump to the next header
                    read(fd,&hd, BLOCKSIZE);
                }
                *nb_block_to_delete -= 1; //not counting the first header (fd starting next to the first header)
                return 1;
            }
        }
        lseek(fd, nb_bloc_fichier*512, SEEK_CUR); //jumping to the next file header
    }
    return -1;//no such file
}

//full_path is the concatenation of PATH and the "target" user wants to delete (rm)
int rm_in_tar(int fd, char* full_path, int arg_r, int first_call){
    int nb_content_bloc = 0;
    off_t file_offset = 0;
    if(rm_in_tar_aux(fd, full_path, &nb_content_bloc,&file_offset, arg_r) < 0 ){
        if(first_call == 1){
            char *msg;
            if(arg_r == 0) msg = "File not found or target is a directory.\n";
            else msg = "Directory not found.\n";
            write(2,msg,strlen(msg));
        }
        return -1;
    }
    struct posix_header hd;
    lseek(fd, file_offset + (nb_content_bloc * 512), SEEK_SET);//positionning fd after all the content blocks of "target"
    while(read(fd, &hd, 512) > 0 ){
        lseek(fd, -nb_content_bloc*512 - 512*2, SEEK_CUR); //repositionning fd back to the block we want to replace
        if(is_a_end_bloc(&hd) != 1){ //if what we read is not a zero byte block
            write(fd, &hd, 512); //overwriting the block by what we read previously
            lseek(fd, nb_content_bloc*512 + 512, SEEK_CUR); //repositionning the offset to the next replacing block
        } else break;
            //reaching a zero byte block meaning the un-replaced blocks will be replaced by zero byte blocks
    }
    //overwriting remaining blocks with zero byte block
    //but if we've already reached the end of the tar earlier nothing will be done
    char zero[512];
    memset(zero,0,512);
    while(read(fd, &hd, BLOCKSIZE) > 0) {
        lseek(fd,(-512), SEEK_CUR);
        write(fd, zero, BLOCKSIZE);
    }
    //procedure to truncate the tar
    off_t final_size = (number_of_block(fd) - nb_content_bloc+1)*512;
    ftruncate(fd, final_size);
    if(arg_r == 0) return 1;
    else{
        //making sure there is no other blocks to delete
        while(rm_in_tar(fd,full_path,arg_r,0) != -1);
        return 1;
     }
}

int option_r_present(char option[50][50], int nb_option){
    for(int i = 0; i < nb_option; i++){
        if(strcmp(option[i],"-R") == 0 || strcmp(option[i],"-r") == 0)
            return 1;
    }
    return 0;
}

char** execvp_opts2(char *dir, char option[50][50], int nb_option){ //fonciton temporaire
    char **ret;
    assert((ret = malloc(sizeof(char*) * (nb_option+3))) != NULL );
    assert((ret[0] = malloc(sizeof(char)*2)) != NULL);
    strcpy(ret[0], "rmdir");
    int index = 1;
    for(int i = 0; i < nb_option; i++){
        assert((ret[index] = malloc(sizeof(char)*strlen(option[i]))) != NULL);
        strcpy(ret[index], option[i]);
        index++;
    }
    if(dir != NULL){
        assert((ret[index] = malloc(sizeof(char)*strlen(dir))) != NULL);
        strcpy(ret[index], dir);
        index++;
    }
    ret[index] = NULL;
    return ret;
}

void exec_rm(char **args){
    int r = fork();
    if(r == 0) execvp("rm", args);
    else wait(NULL);
}

void restoreLastState3(tsh_memory *memory){ //fonction temporaire 
    copyMemory(&old_memory, memory); //restoring the last state of the memory
    char *destination = malloc(strlen(memory->REAL_PATH));
    strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
    cd(destination,memory); //cd-ing back to where we were
    free(destination);
}

int rm(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option){
    char location[512];
    for(int i = 0; i < nb_arg; i++){
        copyMemory(memory, &old_memory);
        getLocation(args[i], location); //check string_traitement for details
        int lenLocation = strlen(location);
        if(lenLocation > 0){//if there is an extra path cd to that path
            if(cd(location, memory) == -1) return -1;
        }
        char *dirToDelete = args[i] + lenLocation;

        if(in_a_tar(memory) == 1){
            char *path_to_target;
            int option_r;
            if((option_r = option_r_present(option, nb_option)) == 1)        
                path_to_target = concatDirToPath(memory->FAKE_PATH, dirToDelete); //concate and adds slash at the end (@string_traitement.c)
            else 
                path_to_target = concate_string(memory->FAKE_PATH, dirToDelete);
            rm_in_tar(atoi(memory->tar_descriptor),path_to_target,option_r,1);
        } else {
            exec_rm(execvp_opts2(dirToDelete, option, nb_option));
        }

        //restoring the last state of the memory
        copyMemory(&old_memory, memory);
        char *destination = malloc(strlen(memory->REAL_PATH));
        strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
        cd(destination,memory); //cd-ing back to where we were
    }
    return 1;
}