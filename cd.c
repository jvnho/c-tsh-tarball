#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/errno.h>
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"

int if_cd_is_valid(int descriptor, char * PATH, char * directory){
    lseek(descriptor, 0, SEEK_SET);
    
    struct posix_header *header = malloc(512);
    int nb_bloc_file = 0;
    char * recherched_path = concatString(PATH, directory);
    
    while(read(descriptor, header, 512)>0){//parcour de tete en tete jusqu' a la fin
        if(strcmp(header->name, recherched_path)==0)return 1;
        int tmp = 0;
        sscanf(header->size, "%o", &tmp);
        nb_bloc_file = (tmp + 512 -1) / 512;
        for(int i=0; i<nb_bloc_file; i++){
            read(descriptor, header, 512);
        }
    }
    return 0;
}
//path and path descriptor
int cd_in_tar(char * directory, char *PATH, char *tar_fd){//modify the current path in the memory
    
    if(strcmp(".",directory)==0)return 0;
    if(strcmp("..", directory)==0)return 0;//todo
    int tar_descriptor = atoi(tar_fd);
    if(if_cd_is_valid(tar_descriptor, PATH, directory)){
        strcat(PATH, concatString(directory, ""));//?? if we give directory/
        return 0;
    }else{
        write(1, "no such directory\n", strlen("no such directory\n"));
        return -1;
    }
    return 0;
}
int cd(char *directory, tsh_memory *memory){
    if(in_a_tar(memory)){//in a anormal circonstances
        return cd_in_tar(directory, memory->FAKE_PATH, memory->tar_descriptor);
    }
    // beforeTar/ directory.tar / afterTar
    char beforTar[512]; char tarName[512]; char afterTar[512];
    //if we have a pre Tar we apply chdir on that part 
    if(strlen(beforTar)){
        if(chdir(beforTar)==-1){
            perror("");
            return -1;
        }
    }
}