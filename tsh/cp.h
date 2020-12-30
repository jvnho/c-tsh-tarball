#ifndef CP_H 
#define CP_H
#include "tsh_memory.h"
extern int copy_tar(char listOption[50][50], char listArgs[50][50], int size_option, int size_args, tsh_memory *memory);
extern int copy(char listOption[50][50], int size_option, char *source, char *real_target, tsh_memory *memory, int r);
#endif