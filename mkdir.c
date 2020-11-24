#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <time.h>
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"
struct posix_header *create_header(char * name){

    struct posix_header *result = malloc(512);
    strcpy(result->name, name);//add the name
    sprintf(result->mode, "000755 ");
    sprintf(result->uid, "000765 ");
    sprintf(result->gid, "000024 ");

    sprintf(result->size, "%011o", 0);
    sprintf(result->mtime, "%011lo", time(NULL));

    result->typeflag = '5';
    result->linkname[0] = '\0';

    strcpy(result->magic, "ustar");
    result->version[0]='0';
    result->version[1]= '0';

    strcpy(result->uname, getlogin());
    strcpy(result->gname, getlogin());

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
            read(descriptor, header, 512);
        }
        j++;
    }
}
void exec_mkdir(char option[50][50], int size_option, char *dir){
    if(size_option == 0){
        printf("dir name = %s\n", dir);
        execlp("mkdir", "mkdir", dir, NULL);
    }
    else if(size_option == 1)execlp("mkdir", "mkdir", option[0], dir, NULL);
    else if(size_option == 2)execlp("mkdir", "mkdir", option[0], option[1], dir, NULL);
    else if(size_option == 3)execlp("mkdir", "mkdir", option[0], option[1], option[2], dir, NULL);
    else execlp("mkdir", "mkdir", option[0], option[1], option[2], option[3], dir, NULL);
}
int mkdir_in_tar(char *dir_name, int tar_descriptor){
    struct posix_header *new_head = create_header(dir_name);
    put_at_the_first_null(tar_descriptor);
    if(write((tar_descriptor), new_head, 512)==-1){//write on the first ending bloc
        perror("");
        return -1;
    }
    //add the last bloc of zero at the end
    lseek(tar_descriptor, 0, SEEK_END);
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    if(write(tar_descriptor, end_bloc, 512)==-1){
        perror("");
        return -1;
    }
    return 0;
}
//to do with more than one argument
int mkdir(char listOption[50][50], char *dir_name, int size_option, tsh_memory *memory){
    printf("---dir name = -%s-\n", dir_name);
    if(in_a_tar(memory)){//in tar -> so use our implementation of mkdir
        return mkdir_in_tar(concatString(memory->FAKE_PATH, dir_name), string_to_int(memory->tar_descriptor));
    }else{//normal circonstances so we exec the normal mkdir
        int pid = fork();
        if(pid==0){//child
            exec_mkdir(listOption, size_option, dir_name);
        }else{//parent
            int status;
            waitpid(pid, &status, WUNTRACED);
            if(WEXITSTATUS(status)==-1)return -1;
        }
    }
    return 0;
}
