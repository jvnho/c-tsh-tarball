#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int is_already_in_array(int,char[20][255],char*);
void print_array_to_STROUT(char[20][255],int);

char ARRAY[20][255]; //allow to keep the file or repository to display
char NAME[255];

#define BUFSIZE 512

int ls_sans_argument(int fd, char* PATH){
    struct posix_header *header = malloc(512);
    int nb_fichier = 0, nb_file_to_display = 0;
    char CUT_PATH[20][255];
    while(read(fd, header, BUFSIZE) > 0){ //reading the entire tarball
        if( (strcmp(PATH," ") == 0) && header->typeflag == '0'){
            //cas précis où on se trouve dans le début du tarball
        } else {
            strncpy(NAME, header->name, strlen(PATH));
            //checking if the current the repository belongs to the current PATH
            //and making sure the current block is not the PATH
            if( (strcmp(NAME,PATH) == 0) && (strcmp(NAME,header->name) != 0) ){
                int i = strlen(PATH), j = 0;
                char *file_path = header->name;
                //splitting the string to keep the name not the full path
                while(file_path[i] != '\0' && file_path[i] != '/' ){
                    CUT_PATH[nb_fichier][j++] = file_path[i++];
                }
                CUT_PATH[nb_fichier][j++] = '\0';
                //making sure the file is display only once
                if( is_already_in_array(nb_file_to_display, ARRAY, CUT_PATH[nb_fichier]) == 0 )
                    memcpy(ARRAY[nb_file_to_display++], CUT_PATH[nb_fichier], strlen(CUT_PATH[nb_fichier]));
                nb_fichier++;
            }
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

int is_already_in_array(int size, char array[20][255], char *string){ //checking if the string is in the list
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

/////// TEST /////////
// int main(int argc, char * argv[]){
//     int fd = open(argv[1], O_RDONLY);
//     int ret = ls_sans_argument(fd,"doc1/doc2/");
// }
