#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "tar.h"
struct posix_header *create_header(char * name){
    
    struct posix_header *result = malloc(512);
    strcpy(result->name, name);//add the name
    sprintf(result->mode, "0000700");
     
    result->uid[0] = '\0';//how to get the id of the creator
    
    result->gid[0] = '\0';//how to get the id of the group

    sprintf(result->size, "%0o", 0);  
    result->mtime[0] = '\0';//how to get the time of the arrchivement
    set_checksum(result);
    result->typeflag = '5';
    result->linkname[0] = '\0';   
    
    strcpy(result->magic, "ustar"); 
    result->version[0]='0';
    result->version[1]= '0'; 
           
    result->uname[0] = '\0';//how to get the user name
    result->gname[0] = '\0';//how to get group name
    
    strcpy(result->devmajor, "000000");
    strcpy(result->devminor, "000000");             
    result->prefix[0] = '\0';             
    result->junk[0]= '\0';  
    return result;
}
void mkdir2(char *dir_name, int tar_descriptor){

    struct posix_header *new_head = create_header(dir_name);
    lseek((tar_descriptor), -(2*512), SEEK_END);//because at the end of a tar file we have 2 null
    char end = '\0';
    write((tar_descriptor), new_head, sizeof(struct posix_header));
    write((tar_descriptor), &end, sizeof(char));
    write((tar_descriptor), &end, sizeof(char));
}
int main(int n, char** args){
    //dir fake ouverture
    int ouverture = open(args[1], O_RDWR);
    mkdir2(args[2], ouverture);
}