#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define BUFSIZE 512

int is_in_array(char*);
void print_ls_to_STROUT(int);
void free_array_of_string();
void fill_info_array(struct posix_header*);

char **ARRAY; //allow to keep the file or repository to display
char **FILE_INFO; // allow to keep file info (i.e size, uname, gname,...) if -l is given as argument
int NUM_FILE = 0; //keeps a track of the size of ARRAY
char FILE_PATH[255], CUT_PATH[255];//FILE_PATH: path of the file , CUT_PATH = filename cut from the path

int ls(int fd, char* PATH, int arg_l){
    int ret_lseek = lseek(fd, 0, SEEK_SET);
    struct posix_header *header = malloc(512);
    if(header == NULL)
        return 0;
    while(read(fd, header, BUFSIZE) > 0){ //reading the entire tarball
        strncpy(FILE_PATH, header->name, strlen(PATH));//splitting the file path to make it matched with the given PATH
        //checking if the current file/repository belongs to the given PATH and if it's not itself (to not print in)
        if(strcmp(FILE_PATH,PATH) == 0 && strcmp(FILE_PATH,header->name) != 0){

            int i = strlen(PATH), j = 0;
            while(header->name[i] != '\0' && header->name[i] != '/' ){
                i++; j++;
            }
            strncpy(CUT_PATH, header->name+strlen(PATH), j); //"cutting" the filepath
            CUT_PATH[j++] = '\0';

            if( is_in_array(CUT_PATH) == 0){ //checking if the file is not is the array (to not print in more than once)
                
                if(arg_l == 1){
                    fill_info_array(header);//making FILE_INFO's array if -l argument is given
                }

                ARRAY = (char**) realloc(ARRAY, (NUM_FILE+1) * sizeof(char*));//reallocating size for ARRAY
                if(ARRAY == NULL) return 0;

                ARRAY[NUM_FILE] = (char*) malloc(255);//allocating size for ARRAY entry
                if(ARRAY[NUM_FILE] == NULL) return 0;

                memcpy(ARRAY[NUM_FILE++], CUT_PATH, strlen(CUT_PATH));//copying CUT_PATH(i.e file/rep name to ARRAY)
            }
        }
        //allow to jump to the next header block
        int filesize = 0;
        sscanf(header->size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, header, BUFSIZE);
    }
    print_ls_to_STROUT(arg_l);
    free(header);
    free_array_of_string();
    return 1;
}

int is_in_array(char *string){ //checking if string is in ARRAY
    for(int i = 0; i < NUM_FILE; i++){
        if(strcmp(string, ARRAY[i]) == 0)
            return 1;
    }
    return 0;
}

void fill_info_array(struct posix_header *header){
    FILE_INFO = (char**) realloc(FILE_INFO,(NUM_FILE+1)*sizeof(char*)); //reallocating size for FILE_INFO
    FILE_INFO[NUM_FILE] = (char*) malloc(255);// allocating space for array entry
    int filesize = 0;
    sscanf(header->size,"%o",&filesize);
    char *c= malloc( (strlen(header->uname)+strlen(header->gname)+strlen(header->size)+12) * sizeof(char));
    sprintf(c, "%s %s %s %d", "----------",header->uname, header->gname, filesize);
    memcpy(FILE_INFO[NUM_FILE], c, strlen(c));
}

void print_ls_to_STROUT(int arg_l){
    if(arg_l == 0){
        for(int i = 0; i < NUM_FILE; i++)
            write(1, strcat(ARRAY[i]," "), strlen(ARRAY[i])+2);
        write(1,"\n",1);
    } else { //print with -l
        for(int i = 0; i < NUM_FILE; i++){
            write(1, strcat(FILE_INFO[i]," "), strlen(FILE_INFO[i])+1);
            write(1, strcat(ARRAY[i]," \n"), strlen(ARRAY[i])+2);
        }
    }
}

void free_array_of_string(){
    for(int i = 0; i < NUM_FILE; i++)
        free(ARRAY[i]);
}

///////// TEST /////////
int main(int argc, char * argv[]){
    int fd = open(argv[1], O_RDONLY);
    int ret = 0;
    if(argv[2] != NULL)
        ret = ls(fd, argv[2], 0);
    else
        ret = ls(fd,"", 1);
}
