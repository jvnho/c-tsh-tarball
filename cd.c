#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "tar.h"
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
void cd(int descriptor, char * PATH, char * directory){
    if(strcmp(".",directory)==0)return;
    if(strcmp("..", directory))return;
    if(if_cd_is_valid(descriptor, PATH, directory)){
        strcat(PATH, concatString(directory, ""));
    }else{
        printf("no such directory\n");
    }
}