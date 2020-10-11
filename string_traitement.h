#ifndef STRING_TRAITEMENT_H
#define STRING_TRAITEMENT_H
extern char ** getCommand(int *nb, char *args);
extern void freeCommand(char **command, int size);
extern int string_to_int(char *chiffre);
#endif