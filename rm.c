#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#include "tar.h"
#include "string_traitement.h"
#include "tsh_memory.h"
#include "cd.h"
#include "exec_funcs.h"

char **array_execvp;

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
    while(read(fd, &hd, 512) > 0)
    {
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        if(strcmp(hd.name, full_path) == 0)
        {
            *debut = lseek(fd, 0, SEEK_CUR);
            if(arg_r == 0)
            {
                if(hd.typeflag != '5'){ //if it's not a directory
                    *nb_block_to_delete = nb_bloc_fichier;
                    return 1;
                }
            } 
            else //argument -r was given as option
            { 
                while(strncmp(hd.name, full_path,strlen(full_path)) == 0)
                {
                    sscanf(hd.size, "%o", &filesize);
                    nb_bloc_fichier = (filesize + 512 -1) / 512;
                    (*nb_block_to_delete) = (*nb_block_to_delete)+ nb_bloc_fichier+1;
                    lseek(fd, nb_bloc_fichier*512, SEEK_CUR);//jump to the next header
                    read(fd,&hd, 512);
                }
                (*nb_block_to_delete) = (*nb_block_to_delete) - 1; //not counting the first header (fd starting next to the first header)
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

    if(rm_in_tar_aux(fd, full_path, &nb_content_bloc,&file_offset, arg_r) < 0)
    {
        if(first_call == 1)
        {
            char *msg;
            if(arg_r == 0) msg = "File not found or target is a directory.\n";
            else msg = "Directory not found.\n";
            write(2,msg,strlen(msg));
        }
        return -1;
    }
    struct posix_header hd;
    lseek(fd, file_offset + (nb_content_bloc * 512), SEEK_SET);//positionning fd after all the content blocks of "target"

    while(read(fd, &hd, 512) > 0)
    {
        lseek(fd, -nb_content_bloc*512 - 512*2, SEEK_CUR); //repositionning fd back to the block we want to replace
        write(fd, &hd, 512); //overwriting the block by what we read previously
        lseek(fd, nb_content_bloc*512 + 512, SEEK_CUR); //repositionning the offset to the next replacing block
    }
    //procedure to truncate the tar
    off_t final_size = (number_of_block(fd) - nb_content_bloc+1)*512; //total_size of the tar minus the number of content blocks (*512) we shift 
    ftruncate(fd, final_size);

    if(arg_r == 0){
        return 1;
    } else {
        //making sure there is no other blocks to delete
        while(rm_in_tar(fd,full_path,arg_r,0) != -1);
        return 1;
     }
}

int do_rm(tsh_memory *memory, char *arg, char option[50][50],int nb_option, int arg_r){
    char location[512];
    tsh_memory old_memory;
    copyMemory(memory, &old_memory);

    getLocation(arg, location); //check string_traitement for details
    int lenLocation = strlen(location);
    char *dirToDelete = arg;

    if(lenLocation > 0){//if there is an extra path cd to that path
        if(cd(location, memory) == -1) 
            return -1;
        dirToDelete = arg + lenLocation;

    }
    if(in_a_tar(memory) == 1)
    {
        char path_to_target[512];
        if(arg_r == 1){        
            concatenationPath(memory->FAKE_PATH, dirToDelete, path_to_target); //concate and adds slash (if needed) (@string_traitement.c)
        } else { 
            concatenation(memory->FAKE_PATH, dirToDelete, path_to_target); //same but doesn't add slash
        }
        rm_in_tar(atoi(memory->tar_descriptor),path_to_target,arg_r,1);
    } else {
        array_execvp = execvp_array("rm", dirToDelete, option, nb_option); //check @function.h
        exec_cmd("rm", array_execvp);
    }
    restoreLastState(old_memory, memory);
    return 1;
}

int rm(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option){
    int option_r = (option_present("-r", option, nb_option) == 1 || option_present("-R", option, nb_option) == 1); //if option -r or -R was given
    for(int i = 0; i < nb_arg; i++){
        if(do_rm(memory, args[i], option, nb_option, option_r) == -1) continue;
    }
    return 1;
}