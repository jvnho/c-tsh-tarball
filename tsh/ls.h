#ifndef LS_H
#define LS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include "tsh_memory.h"

#define NB_ENTRY 512
#define MAX_SIZE 1024

typedef struct ls_memory{
    char NAME[NB_ENTRY][MAX_SIZE]; //allow to keep the file or repository to display
    char INFO[NB_ENTRY][MAX_SIZE]; // allow to keep file info (i.e size, uname, gname,...) if -l is given as argument
    int NUMBER; //keeps a track of the size of ARRAY
} ls_memory;

extern int ls(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50], int nb_option);

#endif