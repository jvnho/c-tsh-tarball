#ifndef STRING_TRAITEMENT_H
#define STRING_TRAITEMENT_H
extern int string_to_int(char *chiffre);
extern char *int_to_string(int chiffre);
extern char * concatString(char * path, char *dir);
extern void getPreTar(char *initial_string, char *result);
extern void getTarName(char *initial_string, char *result);
#endif