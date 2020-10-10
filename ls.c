#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

//int is_already_in_array(int,char**,char*);

#define BUFSIZE 512

char *ARRAY[20]; //allow to keep the file or repository to display
char NAME[255];

int ls_sans_argument(int fd, char* PATH){
    struct posix_header *header = malloc(512);
    int nb_fichier = 0;
    while(read(fd, header, BUFSIZE) > 0){ //reading the entire tarball
        if( (strcmp(PATH," ") == 0) && header->typeflag == '0'){
            //cas où on se trouve dans le début du tarball
        } else {
            strncpy(NAME, header->name, strlen(PATH));
            //checking if the current the repository belongs to the current PATH
            //and making sure the current block is not the PATH
            if( (strcmp(NAME,PATH) == 0) && (strcmp(NAME,header->name) != 0) ){
                //printf("%s\n",header->name);
                char RET_NAME[255];
                int i = strlen(PATH), j = 0;
                char *file_path = header->name;
                while(file_path[i] != '\0' && file_path[i] != '/' ){
                    RET_NAME[j++] = file_path[i++];
                }
                RET_NAME[j++] = '\0';
                printf("%s\n",RET_NAME);
                ARRAY[nb_fichier++] = NAME;
            }
        }
        int filesize = 0;
        sscanf(header->size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, header, BUFSIZE);
    }
    //printf("%s", ARRAY[0]);
    free(header);
    return 1;
}

// int is_already_in_array(int size, char *array[], char *string){ //servira à ne pas avoir des dossiers en double
//     for(int i = 0; i < size; i++){
//         if(strcmp(string, array[i]) == 0)
//             return 1;
//     }
//     return 0;
// }

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
