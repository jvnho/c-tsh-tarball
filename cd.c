#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "tar.h"
#include "tsh_memory.h"
char * concatString(char * path, char *dir){
    int length = strlen(path)+strlen(dir)+2;
    char * result = malloc(length);
    strcpy(result, path);
    strcat(result, dir);
    result[length-2] = '/';
    result[length-1] = '\0';
    return result;
}
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
void cd(char * directory, int path_descriptor, int tar_descriptor){//modify the current path in the memory
    if(strcmp(".",directory)==0)return;
    if(strcmp("..", directory)==0)return;
    //tar desc, path descri, directory
    lseek(path_descriptor, 0, SEEK_SET);
    char PATH[512]; PATH[0] = '\0';
    read(path_descriptor, PATH, 512);
    if(if_cd_is_valid(tar_descriptor, PATH, directory)){
        //read_the actual path 
        strcat(PATH, concatString(directory, ""));//?? if we give directory/
        lseek(path_descriptor, 0, SEEK_SET);
        write(path_descriptor, PATH, strlen(PATH));
    }else{
        printf("no such directory\n");
    }
}