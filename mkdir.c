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
#include "cd.h"

tsh_memory old_memory; //will be use to save/restore a memory
char location[512];

void exec_mkdir(char option[50][50], int size_option, char *dir){
    if(size_option == 0){
        execlp("mkdir", "mkdir", dir, NULL);
    }
    else if(size_option == 1)execlp("mkdir", "mkdir", option[0], dir, NULL);
    else if(size_option == 2)execlp("mkdir", "mkdir", option[0], option[1], dir, NULL);
    else if(size_option == 3)execlp("mkdir", "mkdir", option[0], option[1], option[2], dir, NULL);
    else execlp("mkdir", "mkdir", option[0], option[1], option[2], option[3], dir, NULL);
}
int mkdir_in_tar(char *dir_name, int tar_descriptor){
    if(dir_exist(tar_descriptor, dir_name))return 0;
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
int makeDirectory(char listOption[50][50], char *dir_name, int size_option, tsh_memory *memory){
    copyMemory(memory, &old_memory);
    getLocation(dir_name, location);
    int lenLocation = strlen(location);
    char *dirToCreate = dir_name;
    if(lenLocation){//if there is an extra path cd to that path
        if(cd(location, memory)==-1){
            return -1;
        }
        dirToCreate = dir_name + lenLocation;
    }
    int result= 0;
    char *destination;
    if(in_a_tar(memory)){//in tar -> so use our implementation of mkdir
        result = mkdir_in_tar(concatDirToPath(memory->FAKE_PATH, dirToCreate), string_to_int(memory->tar_descriptor));
        copyMemory(&old_memory, memory);
        destination = malloc(strlen(memory->REAL_PATH));
        strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
        cd(destination,memory);
        return result;
    }else{//normal circonstances so we exec the normal mkdir
        int pid = fork();
        if(pid==0){//child
            exec_mkdir(listOption, size_option, dirToCreate);
        }else{//parent
            copyMemory(&old_memory, memory);
            destination = malloc(strlen(memory->REAL_PATH));
            strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
            cd(destination,memory);
            int status;
            waitpid(pid, &status, WUNTRACED);
            if(WEXITSTATUS(status)==-1)return -1;
        }
    }
    return 0;
}
int mkdir(char listOption[50][50], char listArgs[50][50], int size_option, int size_args, tsh_memory *memory){
    int nb_created = 0;
    for(int i = 0; i<size_args; i++){
        if(makeDirectory(listOption, listArgs[i], size_option, memory)==0)nb_created++;
    }
    return nb_created > 0 ? 0: -1;
}
