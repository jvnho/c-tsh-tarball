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
tsh_memory * instanciate_tsh_memory(char *tar_file_name){
    tsh_memory * result = malloc(sizeof(tsh_memory));
    (result->PATH)[0] = '\0';//so it doesn't create a random characteres
    result->tar_descriptor = int_to_string(open(tar_file_name, O_RDWR));
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
    freeCommand(state->comand, state->tail_comand);

}
void free_tsh_memory(tsh_memory *state){//at the end
    close(atoi(state->tar_descriptor));
    free(state);
}
void getCommand(char **command_typed, tsh_memory *memory){
    memory->comand = command_typed;
}
