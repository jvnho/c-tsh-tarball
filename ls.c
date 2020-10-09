#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"

#define BUFSIZE 512
char *BUFFER[BUFSIZE];

struct posix_header *tete = malloc(512);

void ls_sans_argument(int fd, char* PATH){
    while(read(fd, tete, BUFSIZE) > 0){
        if( memmem(tete->name, strlen(tete->name), PATH, strlen(PATH)) != NULL){
        } else {
            for(int i = 0; i < (tete->size + 512 - 1) / 512; i++)
                read(fd, tete, BUFSIZE);
        }
    }
}

// char *cut_path(char* PATH, char* PATH_TO_CUT){
//
// }

void ls(int fd, char* PATH int arg, char* argv){
    int ret_lseek = lseek(fd, 0, SEEK_SET);
    if(arg == 0){
        ls_sans_argument(fd,PATH);
    }/* else {
        ls avec option -l et/ou avec argument
    }*/
}
