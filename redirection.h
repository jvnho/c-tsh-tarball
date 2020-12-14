#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "tsh_memory.h"

#define NB_ENTRY 512
#define MAX_SIZE 1024

typedef struct redirection_array{
    char OUTPUT_NAME[NB_ENTRY][MAX_SIZE]; //name of the redirection target
    int STD[NB_ENTRY];// 0 stdin, 1 stdout, 2 strerr, 3 both(2>&1)
    int APPEND[NB_ENTRY]; //1 or 0
    int NUMBER; //number of redirection
} redirection_array;

extern int redirection(tsh_memory *memory);

#endif