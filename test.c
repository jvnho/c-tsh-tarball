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


// write like printf("")
    void display(char* str){
        write(1,str,sizeof(char)*strlen(str));

    }
    


char myPath[256]; //FILE_PATH of my array 256  


int cat(int desc, char* path, int arg){ // I should use pointer rather than int next time
    lseek(desc, 0, SEEK_SET);

    struct posix_header *header = malloc(512); // 

    if(header == NULL){
        return 0;

    } 

   if(path != NULL && path[0]!= 32 && path[0] != 10){ // ascii code of Line Feed(saut de line) and space

    while(read(desc, header, BLOCKSIZE) > 0){ // blocksize = 512

        strncpy(myPath, header->name, strlen(path));
        if(strcmp(myPath,path) == 0 && strcmp(myPath,header->name) != 0){ 
            int file_s = 0;
        sscanf(header->size, "%o", &file_s);
        int nb_bloc_fichier = (file_s + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++){
            read(desc, header, BLOCKSIZE);
            //condition
            display(header); //display
            

        } 
       
             
        }
    //else 
    else{
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

    //display all without condition
    char myStr[255];
    read(0,&myStr, sizeof(myStr));
    while(myStr[0]!= 'Q' ){ //exit
        int myRead= read(0,&myStr, sizeof(myStr));
        if (myRead != 0){
            write(1,&myStr,sizeof(myStr));

        }
        
        

    }


        return 0;
    }

    





