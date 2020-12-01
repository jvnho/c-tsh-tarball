#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tar.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#include "tar.h"
#include "tsh_memory.h"
#include "rmdir.h"
#include "cd.h"
#include "string_traitement.h"

tsh_memory old_memory;

//returns the number of times the path appears in the tarball
//returns to a pointer 'file_offset' the position of the rep and the number of blocks found
int occ_counter_path(int fd, char* full_path, off_t* file_offset, off_t *tar_size){
    lseek(fd, 0, SEEK_SET);
    int occurence = 0;
    struct posix_header hd;
    while(read(fd, &hd, 512) > 0){//reading the entire tarball
        if(strncmp(hd.name, full_path, strlen(full_path)) == 0){
            occurence++;
            if(hd.typeflag == '5'){
                *file_offset = lseek(fd,0,SEEK_CUR);//position of the blocks RIGHT NEXT to the one the user wants to delete
            }
        }
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_content_block = (filesize + 512 -1) / 512;

        (*tar_size) += (1+nb_content_block)*512;
        lseek(fd,nb_content_block*512, SEEK_CUR); //allow to jump to the next header block
    }
    return occurence;
}

int rmdir_in_tar(int fd, char* full_path){
    struct posix_header hd;
    off_t file_offset = 0, final_size = 0; //offset will allow to start reading from a certain position
    int occurrence = occ_counter_path(fd, full_path, &file_offset, &final_size);
    if(occurrence != 1){
        char s[] = "Directory is not empty or does not exist.\n";
        write(2, s, strlen(s));
        return -1;
    }
    // we found one AND only ONE block so we proced to shift blocks
    lseek(fd,file_offset, SEEK_SET); //starting from the end of the block we want to delete
    while(read(fd, &hd, BLOCKSIZE) > 0){
        lseek(fd, (-BLOCKSIZE*2), SEEK_CUR); //going back to the last block
        write(fd, &hd, BLOCKSIZE); //overwriting the block
        lseek(fd, BLOCKSIZE, SEEK_CUR); //repositionning the offset to the next replacing block
    }
    //procedure to truncate the tar
    lseek(fd,0,SEEK_SET);
    ftruncate(fd, final_size-512); // -512 because we replace a header by a zero byte block so we can delete it
    return 1;
}

char** execvp_opts(char *dir, char option[50][50], int nb_option){ //fonciton temporaire
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

void exec_rmdir(char **args){
    int r = fork();
    if(r == 0) execvp("rmdir", args);
    else wait(NULL);
}

void restoreLastState2(tsh_memory *memory){ //fonction temporaire 
    copyMemory(&old_memory, memory); //restoring the last state of the memory
    char *destination = malloc(strlen(memory->REAL_PATH));
    strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
    cd(destination,memory); //cd-ing back to where we were
    free(destination);
}

int rmdir_func(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option){
    char location[512];
    for(int i = 0; i < nb_arg; i++){

        copyMemory(memory, &old_memory);

        getLocation(args[i], location); //check string_traitement for details
        int lenLocation = strlen(location);
        if(lenLocation > 0){//if there is an extra path cd to that path
            if(cd(location, memory) == -1) return -1; //path doesn't exist
        }
        char *dirToDelete = args[i] + lenLocation;
        
        if(in_a_tar(memory) == 1 && is_unix_directory(dirToDelete) == 0){
            char *path_to_dir = concatDirToPath(memory->FAKE_PATH, dirToDelete);
            rmdir_in_tar(atoi(memory->tar_descriptor),path_to_dir);
        } else {
            if(strcmp(option[0],"-p")==0){ //cas spécial du rmdir du système unix 
                restoreLastState2(memory);
                exec_rmdir(execvp_opts(args[i],option, nb_option));
            } else { 
                exec_rmdir(execvp_opts(dirToDelete,option, nb_option));
            }
        }
        restoreLastState2(memory);
    }
    return 1;
}
