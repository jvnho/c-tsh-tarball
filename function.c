#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tsh_memory.h"
#include "cd.h"

void restoreLastState(tsh_memory old_memory, tsh_memory *memory){
    copyMemory(&old_memory, memory); //restoring the last state of the memory
    char *destination = malloc(strlen(memory->REAL_PATH));
    strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
    cd(destination,memory); //cd-ing back to where we were
    free(destination);
}