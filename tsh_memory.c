#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "tsh_memory.h"
tsh_memory * instanciate_tsh_memory(int path_descriptor, int tar_descriptor){
    tsh_memory * result = malloc(sizeof(tsh_memory));
    result->PATH = malloc(BUFSIZE);
    (result->PATH)[0] = '\0';//so it doesn't create a random characteres
    result->path_descriptor = path_descriptor;
    result->tar_descriptor = tar_descriptor;
    return result;
}
void update_path(tsh_memory * state){
    lseek(state->path_descriptor, 0, SEEK_SET);
    read(state->path_descriptor, state->PATH, BUFSIZE);
}
void free_tsh_memory(tsh_memory *state){
    close(state->path_descriptor);
    close(state->tar_descriptor);
    free(state->PATH);
    free(state);
}
