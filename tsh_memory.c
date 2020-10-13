#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <stdio.h>
#include "tsh_memory.h"
#include "string_traitement.h"
tsh_memory * instanciate_tsh_memory(char *path_file_name, char *tar_file_name){
    tsh_memory * result = malloc(sizeof(tsh_memory));
    result->PATH = malloc(BUFSIZE);
    (result->PATH)[0] = '\0';//so it doesn't create a random characteres
    result->path_descriptor = open(path_file_name, O_RDWR);
    result->tar_descriptor = open(tar_file_name, O_RDWR);
    if(errno == ENOENT){//no such file
        perror("");
        return NULL;
    }if(errno == EACCES){//permision dinied
        perror("");
        return NULL;
    }
    return result;
}
void update_path(tsh_memory * state){//change the path variable according to the path file
    
    lseek(state->path_descriptor, 0, SEEK_SET);
    read(state->path_descriptor, state->PATH, BUFSIZE);
    freeCommand(state->comand, state->tail_comand);
}
void free_tsh_memory(tsh_memory *state){
    close(state->path_descriptor);
    close(state->tar_descriptor);
    free(state->PATH);
    free(state);
}
void getCommand(char **command_typed, tsh_memory *memory){
    memory->comand = command_typed;
}
