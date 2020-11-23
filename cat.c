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

#include "tar.h"
#include "tsh_memory.h"
#include "ls.h"

void display(char *str);
void cat_all();
int cat(tsh_memory *memory);
void cat_in_tar(int desc, char* path);



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

int cat(tsh_memory *memory){
    if(in_a_tar(memory) == 1){
        cat_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH);

   }
   else{
       cat_all();
       /*
        for(int i = 0; i < arg; i++){
            cat_in_tar(desc, args[i]);
            */
   }

   return 1;
}


/*
int cat(int desc, char **args, int arg){

    if(arg == 0){
        //call cat_all
        cat_all();
    } else {
        for(int i = 0; i < arg; i++){
            cat_in_tar(desc, args[i]);
        }
    
    }
}
*/

void cat_in_tar(int desc, char* path){ 
    lseek(desc, 0, SEEK_SET);

    struct posix_header header; //

    if(&header == NULL){
        return 0;

    }

    while(read(desc, &header, BLOCKSIZE) > 0){ // blocksize = 512
            int filesize = 0;
            sscanf(header.size, "%o", &filesize);
            int nb_fichier = (filesize + 512-1) /512;
        
            
            if(strcmp(header.name, path)==0){
                char buffer[nb_fichier*512];
                read(desc, buffer,nb_fichier);
                write(1, buffer, strlen(buffer));
                //display(header);
                //condition
                //display(header); //display
            } 
            lseek(desc,nb_fichier*512, SEEK_CUR);
            }
            cat_all();
        
    }


/*
// write like printf("")
    void display(char* str){
        write(1,str,sizeof(char)*strlen(str));

    }
    

char myPath[256]; // path of my array 256  


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
            int myFile = 0;
        sscanf(header->size, "%o", &myFile);
        int nb_bloc_fichier = (myFile + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++){
            read(desc, header, BLOCKSIZE);
            //condition
            display(header); //display
            

        } 
       
             
        }
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
    */