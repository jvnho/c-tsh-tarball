#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"



char **ARRAY; //allow to keep the file or repository to display
int NUM_FILE = 0; //keeps a track of the size of ARRAY
char FILE_PATH[255]; //FILE_PATH


int cat(int desc, char* PATH, int arg){
    lseek(desc, 0, SEEK_SET);

    struct posix_header *header = malloc(512);

    if(header == NULL){
        return 0;

    } 

    while(read(desc, header, BLOCKSIZE) > 0){ // blocksize = 512

        strncpy(FILE_PATH, header->name, strlen(PATH));
        if(strcmp(FILE_PATH,PATH) == 0 && strcmp(FILE_PATH,header->name) != 0){

        
                ARRAY = (char**) realloc(ARRAY, (NUM_FILE+1) * sizeof(char*));
                if(ARRAY == NULL) return 0;

                ARRAY[NUM_FILE] = (char*) malloc(255);
                if(ARRAY[NUM_FILE] == NULL) return 0;

               
             }
        }
        //jump to the next header block
        int file_s = 0;
        sscanf(header->size, "%o", &file_s);
        int nb_bloc_fichier = (file_s + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++){
            read(desc, header, BLOCKSIZE);

        } 
    }
    





