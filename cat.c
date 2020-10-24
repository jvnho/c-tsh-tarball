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

char myPath[256]; // path of my array 256

// write like printf("")
    void display(char* str){
        write(1,str,sizeof(char)*strlen(str));

    }

    void cat_all(){ 
        char myStr[255];
    read(0,&myStr, sizeof(myStr));
    while(myStr[0]!= 'Q' ){ //exit
        int myRead= read(0,&myStr, sizeof(myStr));
        if (myRead != 0){
            write(1,&myStr,sizeof(myStr));

        }
    }

    }




int cat(int desc, char **args, int arg){
    if(arg == 0){
        //call cat_all
        cat_all();
    } else {
        for(int i = 0; i < arg; i++){
            cat_2(desc, args[i]);
        }
    }
}

int cat_2(int desc, char* path){ // I should use pointer rather than int next time
    lseek(desc, 0, SEEK_SET);

    struct posix_header *header = malloc(512); //

    if(header == NULL){
        return 0;

    }

   if(path != NULL){ 

    while(read(desc, header, BLOCKSIZE) > 0){ // blocksize = 512
        if(header-> typeflag == '0'){//if file pointed by header is a ordinary file
            strncpy(myPath, header->name, strlen(path));
            if(strcmp(myPath,path) == 0 && strcmp(myPath,header->name) != 0){
                int myFile = 0;
                sscanf(header->size, "%o", &myFile);
                char buffer[*header->size];
                read(desc, buffer, &myFile);
                write(1, buffer, strlen(buffer));
                //display(header);
                //condition
                //display(header); //display
            } else {
                //jump to the next header block
                int file_s = 0;
                sscanf(header->size, "%o", &file_s);
                int nb_bloc_fichier = (file_s + 512 -1) / 512;
                for(int i = 0; i < nb_bloc_fichier; i++){
                    read(desc, header, BLOCKSIZE);
                //sans afficher car out of condition
                }
            }
        }
    }

    //display all without conditions
    cat_all();

        return 0;

   }
    }
