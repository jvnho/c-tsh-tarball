#ifndef STRING_TRAITEMENT_H
#define STRING_TRAITEMENT_H
extern int count_args(char *args);
extern int string_to_int(char *chiffre);
extern int get_prev_directory(char *path);
extern char *int_to_string(int chiffre);
extern char * concatString(char * path, char *dir);
extern void getPreTar(char *initial_string, char *result);
extern void getTarName(char *initial_string, char *result);
extern void getPostTar(char *initial_string, char *result);
extern char* octal_to_string(char *mode);
extern int getFirstDir(char *source, char *result);
extern char **addNullEnd(char **initial, int size);
extern void remove_simple_dot_from_dir(char *str);
extern void getLocation(char *source, char *result);
#endif
