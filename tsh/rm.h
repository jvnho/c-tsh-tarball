#ifndef RM_H
#define RM_H

#include "tsh_memory.h"

extern int rm(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option);
extern int rm_in_tar(int fd, char* full_path, int arg_r, int first_call);
extern int do_rm(tsh_memory *memory, char *arg, char option[50][50],int nb_option, int arg_r);
#endif
