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

extern int ls(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50], int nb_option);

#endif
