#include <string.h>
#include <unistd.h>
#include "pwd.h"
int pwd(tsh_memory *state){
    char * path = getPath(state);
    write(1, path, strlen(path));//write it on the terminal
    write(1, "\n", strlen("\n"));
    return 0;
}