#ifndef MKDIR_H
#define MKDIR_H
extern void put_at_the_first_null(int descriptor);
extern int mkdir_tar(char listOption[50][50], char listArgs[50][50], int size_option, int size_args, tsh_memory *memory);
#endif