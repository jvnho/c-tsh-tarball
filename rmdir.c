#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tar.h>
#include <unistd.h>

#include "tar.h"

#define BLOCKSIZE 512

char FILE_PATH[512];//will allow to save the path and ONLY the path of the file pointed by the posix_header

int occ_counter_path(int fd, char* full_path){//counts the number of times the path appears in the tarball
    lseek(fd, 0, SEEK_SET);
    int occurence = 0;
    struct posix_header *header = malloc(512);
    if(header == NULL) return 0;
    while(read(fd, header, 512) > 0){//reading the entire tarball
        if(header->typeflag == '5'){ //if the file pointed by posix_header is a repository
            //strncpy(FILE_PATH, header->name, strlen(full_path));
            if(strcmp(FILE_PATH, full_path) == 0)
                occurence++;
        }
        //allow to jump to the next header block
        int filesize = 0;
        sscanf(header->size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, header, BLOCKSIZE);
    }
    return occurence;
}

char *concate_path_rep(char *PATH, char *rep){//temporary function, will be using the function concateString located in string_traitement.c
    char *s = malloc((strlen(PATH) + strlen(rep) + 1) * sizeof(char));
    s[0] = '\0';
    sprintf(s,"%s%s%c", PATH, rep,'\0');//concates the PATH and the rep name !!CHECK IF SPRINTF IS NULL TERMINATED!!
    return s;
}

int rmdir_func(int fd, char* PATH, char *rep){
    if(rep[strlen(rep)-2] != '/') return 0; //rep given is not written as a repository
    char *full_path = concate_path_rep(PATH,rep);
    if(occ_counter_path(fd,full_path) != 1) return 0; //if the repository is not empty or not found then returns 0
    return 0;
}
