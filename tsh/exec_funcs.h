#ifndef EXEC_FUNCS_H
#define EXEC_FUNCS_H

#include "tsh_memory.h"

extern void restoreLastState(tsh_memory old_memory, tsh_memory *memory);
extern char** execvp_array(char *cmd, char *dir, char option[50][50], int nb_option);
extern void exec_cmd(char *cmd, char **args);
extern int option_present(char *opt_name, char option[50][50], int nb_option);
#endif
