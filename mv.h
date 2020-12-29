#ifndef MV_H
#define MV_H

#include "tsh_memory.h"

extern int mv(tsh_memory *mem, char args[50][50], int nb_args, char option[50][50], int nb_opt);
extern int do_mv(tsh_memory *memory, char *src, char *target, char option[50][50], int nb_opt);

#endif