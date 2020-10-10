#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define BUFSIZE 512
char *ARRAY[20]; // (4096/255): 4096 number of characters max fitting in a string so is 255 for a filename

int ls_sans_argument(int fd, char* PATH){
    struct posix_header *tete = malloc(512);
    while(read(fd, tete, BUFSIZE) > 0){
        if(strstr(tete->name, PATH)) { //checks if filename contains PATH (à améliorer)
            int i = strlen(PATH);
            char *file_path = tete->name;
            while(file_path[i] != '\0' && file_path[i] != '/' ){
                write(1, &file_path[i] ,1);
                i++;
            }
            write(1, " " ,1);
        } else { //allows to jump to the next block
            int tmp = 0;
            sscanf(tete->size, "%o", &tmp);
            int nb_bloc_fichier = (tmp + 512 -1) / 512;
            for(int i = 0; i < nb_bloc_fichier; i++)
                read(fd, tete, BUFSIZE);
        }
    }
    write(1, "\n" ,1);
    return 1;
}

int main(int argc, char * argv[]){
    int fd = open(argv[1], O_RDONLY);
    int ret = ls_sans_argument(fd,"doc1/");
}

int cut_path_tok(char* PATH){
    char *token = strtok(PATH, "/");
    int ret = 0;
    while(token != NULL)
        ret++;
    return ret;
}

void ls(int fd, char* PATH, int arg, char* argv){
    int ret_lseek = lseek(fd, 0, SEEK_SET);
    if(arg == 0){
        ls_sans_argument(fd,PATH);
    }/* else {
        ls avec option -l et/ou avec argument
    }*/
}
