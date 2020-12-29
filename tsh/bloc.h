#ifndef BLOC_H
#define BLOC_H
#include <stdio.h>
#include <stdlib.h>
#include "tar.h"
typedef struct content_bloc{
    struct posix_header hd;//header
    char content[512][512];
    int nb_bloc;
}content_bloc;
    //creer un tableau de content_bloc
extern int fill_fromTar(content_bloc *tab, char *source, char *target, int descriptor, char *fake_path, int *starting_index);
extern int fill_fromFile_outside(content_bloc *tab, char *source, char *target, int* starting_index);//file from outside
extern int fill_fromDir_outside(content_bloc *tab, char *directory, int* starting_index);


#endif