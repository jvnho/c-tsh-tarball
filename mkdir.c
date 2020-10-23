#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "tar.h"
struct posix_header *create_header(char * name){
    
    struct posix_header *result = malloc(512);
    strcpy(result->name, name);//add the name
    sprintf(result->mode, "000755 ");
    sprintf(result->uid, "000765 ");
    sprintf(result->gid, "000024 ");

    sprintf(result->size, "%011o", 0); 
    sprintf(result->mtime, "%011o", 1603298969);
     
    result->mtime[0] = '\0';//how to get the time of the arrchivement
    
    result->typeflag = '5';
    result->linkname[0] = '\0';   
    
    strcpy(result->magic, "ustar"); 
    result->version[0]='0';
    result->version[1]= '0'; 

    strcpy(result->uname, "sarobidy");    
    strcpy(result->gname, "staff");
    
    strcpy(result->devmajor, "000000 ");
    strcpy(result->devminor, "000000 ");             
    result->prefix[0] = '\0';             
    result->junk[0]= '\0';  
    set_checksum(result);
    return result;
}
int end_bloc(struct posix_header *header){
    //create a string that has all the bloc zero byte, in order to compare with header
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    //then compare
    if(memcmp(header, end_bloc, 512) == 0)return 1;//identical
    return 0;
}
void put_at_the_first_null(int descriptor){
    lseek(descriptor, 0, SEEK_SET);
    struct posix_header *header = malloc(512);
    int nb_bloc_file = 0;
    int j = 1;
    while(read(descriptor, header, 512)>0){
        if(end_bloc(header)){//the moment we check the first bloc, we return to the back because we have read it
            lseek(descriptor, -512, SEEK_CUR);
            return;
        }
        int tmp = 0;
        sscanf(header->size, "%o", &tmp);
        nb_bloc_file = (tmp + 512 -1) / 512;
        for(int i=0; i<nb_bloc_file; i++){
            printf("   %d em bloc contenu\n", i+1);
            read(descriptor, header, 512);
        }
        j++;
    }
}
void mkdir2(char *dir_name, int tar_descriptor){
    struct posix_header *new_head = create_header(dir_name);
    put_at_the_first_null(tar_descriptor);
    write((tar_descriptor), new_head, 512);//write on the first ending bloc
    
    //add the last bloc of zero at the end
    lseek(tar_descriptor, 0, SEEK_END);
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    write(tar_descriptor, end_bloc, 512);
    //affiche(tar_descriptor);

}
int main(int n, char** args){
    //directory/ nomDuTar
    int ouverture = open(args[2], O_RDWR );
    mkdir2(args[1], ouverture);
}