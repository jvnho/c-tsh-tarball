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
void instanciate_tsh_memory(char *tar_file_name, tsh_memory *result){
    (result->FAKE_PATH)[0] = '\0';//so it doesn't create a random characteres
    result->tar_name = tar_file_name;
    result->tar_descriptor = int_to_string(open(tar_file_name, O_RDWR));
    if(errno == ENOENT){//no such file
        perror("");
    }if(errno == EACCES){//permision dinied
        perror("");
    }
}
tsh_memory * create_memory(){
    tsh_memory * result = malloc(sizeof(tsh_memory));
    (result->FAKE_PATH)[0] = '\0';//so it doesn't create a random characteres
    return result;
}
void free_tsh_memory(tsh_memory *state){//at the end
    //close(string_to_int(state->tar_descriptor)); 
    free(state);
}
