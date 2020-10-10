#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define BUFSIZE 512

char *ARRAY[20];
char NAME[255];

int ls_sans_argument(int fd, char* PATH){
    struct posix_header *tete = malloc(512);
    int nb_fichier = 0;
    while(read(fd, tete, BUFSIZE) > 0){
        strncpy(NAME, tete->name, strlen(PATH)); //allows to check if the current
        if(strcmp(NAME,PATH) == 0){            //the file belongs to the tree file(=PATH)
            int i = strlen(PATH), j = 0;
            if(strcmp(NAME,tete->name) != 0){ //dont want to show the current repository we are in
                printf("%c \n",tete->name[i]);
                char *file_path = tete->name;
                while(file_path[i] != '\0' && file_path[i] != '/' ){
                    NAME[j] = file_path[i];
                    i++; j++;
                }
                NAME[j++] = '\0';
                ARRAY[nb_fichier++] = NAME;
                //printf("%s", ARRAY[1]);
            }
        } else { //allows to jump to the next block
            int filesize = 0;
            sscanf(tete->size, "%o", &filesize);
            int nb_bloc_fichier = (filesize + 512 -1) / 512;
            for(int i = 0; i < nb_bloc_fichier; i++) read(fd, tete, BUFSIZE);
        }
    }
    free(tete);
    return 1;
}

int main(int argc, char * argv[]){
    int fd = open(argv[1], O_RDONLY);
    int ret = ls_sans_argument(fd,"doc1/");
}

void ls(int fd, char* PATH, int arg, char* argv){
    int ret_lseek = lseek(fd, 0, SEEK_SET);
    if(arg == 0){
        ls_sans_argument(fd,PATH);
    }/* else {
        ls avec option -l et/ou avec argument
    }*/
}
