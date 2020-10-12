#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int is_in_array(int,char[20][255],char*);
void print_array_to_STROUT(char[20][255],int);

char ARRAY[20][255]; //allow to keep the file or repository to display

#define BUFSIZE 512

int ls_sans_argument(int fd, char* PATH){
    struct posix_header *header = malloc(512);
    int nb_file_to_display = 0;
    char FILE_PATH[255], CUT_PATH[255];
    while(read(fd, header, BUFSIZE) > 0){ //reading the entire tarball
        strncpy(FILE_PATH, header->name, strlen(PATH));//splitting the file path to make it matched with the given PATH
        //checking if the current file/repository belongs to the given PATH and if it's not itself (to not print in)
        if(strcmp(FILE_PATH,PATH) == 0 && strcmp(FILE_PATH,header->name) != 0){
            int i = strlen(PATH), j = 0;
            while(header->name[i] != '\0' && header->name[i] != '/' ){ //keeping the name of the file path
                i++; j++;
            }
            strncpy(CUT_PATH, header->name+strlen(PATH), j);
            CUT_PATH[j++] = '\0';
            if( is_in_array(nb_file_to_display, ARRAY, CUT_PATH) == 0 ) //checking if the file is not is the array (to not print in more than once)
                memcpy(ARRAY[nb_file_to_display++], CUT_PATH, strlen(CUT_PATH));
        }
        //allow to jump to the next header block
        int filesize = 0;
        sscanf(header->size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, header, BUFSIZE);
    }
    print_array_to_STROUT(ARRAY,nb_file_to_display);
    free(header);
    return 1;
}

int is_in_array(int size, char array[20][255], char *string){ //checking if the string is in the list
    for(int i = 0; i < size; i++){
        if(strcmp(string, array[i]) == 0)
            return 1;
    }
    return 0;
}

void print_array_to_STROUT(char array[20][255], int size){
    for(int i = 0; i < size; i++)
        write(1, strcat(array[i]," "), strlen(array[i])+1);
    write(1,"\n",1);
}

void ls(int fd, char* PATH, int arg, char* argv){
    int ret_lseek = lseek(fd, 0, SEEK_SET);
    if(arg == 0){
        ls_sans_argument(fd,PATH);
    }
    /* else {
        ls avec option -l et/ou avec argument
    }*/
}

/// TEST /////////
// int main(int argc, char * argv[]){
//     int fd = open(argv[1], O_RDONLY);
//     int ret = ls_sans_argument(fd,"");
// }
