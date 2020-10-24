#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tar.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tar.h"

char FILE_PATH[512];//will allow to save the path and ONLY the path of the file pointed by the posix_header

int occ_counter_path(int fd, char* PATH, int* file_offset){//returns the number of times the path appears in the tarball and returns to a pointer 'file_offset' the position of the rep
    lseek(fd, 0, SEEK_SET);
    int occurence = 0;
    int read_length = 0;
    struct posix_header hd;
    while(read_length = read(fd, &hd, 512) > 0){//reading the entire tarball
        strncpy(FILE_PATH, hd.name, strlen(PATH));
        if(strcmp(FILE_PATH, PATH) == 0){
            *file_offset = read_length;
            occurence++;
        }
        //allow to jump to the next header block
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, &hd, BLOCKSIZE);
    }
    return occurence;
}

int rmdir_func(int fd, char* PATH, char *rep){
    //if(rep[strlen(rep)-2] != '/') return 0; //rep given is not written as a repository
    int file_offset = 0;//will allow to start reading the tarball from the file and not the beginning of the tarball
    // if(occ_counter_path(fd,PATH,&file_offset) != 1) return 0; //if the repository is not empty or not found then returns 0
    // char zero[512];
    // memset(zero,0,512);
    // write(fd,zero,512);
    printf("%d\n", occ_counter_path(fd,PATH,&file_offset));
    /* PROCEDURE TO DELETE THE HEADER AND FILE BLOCKS */

    return 1;
}

int main(int argc, char **argv){
    int fd = open(argv[1], O_RDWR);
    rmdir_func(fd,"",argv[2]);
}
