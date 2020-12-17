#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "tsh_memory.h"

#define NB_ENTRY 512
#define MAX_SIZE 1024

typedef struct redirection_array{
    char NAME[NB_ENTRY][MAX_SIZE]; //Name of the redirection target
    int STD[NB_ENTRY];// 0 stdin, 1 stdout, 2 strerr, 3 both(2>&1)
    int APPEND[NB_ENTRY]; //1 or 0
    int IN_A_TAR[NB_ENTRY]; //If the user wants the redirection file to be in a tar
    char REDIR_PATH[NB_ENTRY][MAX_SIZE]; //Give absolute path of (future) redirection file location
    int NUMBER; //Number of redirection (at most 2 redirection a the same time)
} redirection_array;

extern int redirection(tsh_memory *memory);

#endif