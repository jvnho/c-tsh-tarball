#ifndef STRING_TRAITEMENT_H
#define STRING_TRAITEMENT_H
extern char ** to_array_of_string(int *nb, char *args, char *path_fd, char *tar_fd);
extern int string_to_int(char *chiffre);
extern char *int_to_string(int chiffre);
extern char * concatString(char * path, char *dir);
#endif