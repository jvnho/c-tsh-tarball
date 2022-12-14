#ifndef STRING_TRAITEMENT_H
#define STRING_TRAITEMENT_H
#include "tsh_memory.h"
extern int count_args(char *args);
extern int string_to_int(char *chiffre);
extern int get_prev_directory(char *path);
extern char *int_to_string(int chiffre, char *result);
extern char * simpleConcat(char *path, char *dir);
extern void concatDirToPath(char * path, char *dir, char *result);
extern char *concate_string(char *s1, char *s2);
extern void getPreTar(char *initial_string, char *result);
extern void getTarName(char *initial_string, char *result);
extern void getPostTar(char *initial_string, char *result);
extern void octal_to_string(char *mode, char *result);
extern int getFirstDir(char *source, char *result);
extern char **addNullEnd(char **initial, int size);
extern void remove_simple_dot_from_dir(char *str);
extern void getLocation(char *source, char *result);
extern int is_unix_directory(char *str);
extern int is_extension_tar(char *str);
extern int spilitPipe(tsh_memory *source, tsh_memory *memory1, tsh_memory *memory2);
extern void get_tar_path(tsh_memory *memory, char *abs_path, char *container);
extern void concatenationPath(char *first, char *second, char *result);
extern void concatenation(char *first, char *second, char *result);

#endif
