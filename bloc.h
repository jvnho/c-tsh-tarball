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
extern int fill_fromTar(content_bloc *tab, char *source, char *target, int descriptor, char *fake_path);
extern int fill_fromFile(content_bloc *tab, char *source, int starting_index);//file from outside
//int fill_fromDir(content_bloc *tab, char *cheminRelatif_depuis dossier, char *dir)//recursif avec la recherche de fichier par les liste inoeud
    //convertir content_bloc en un header
//header getHeader_from_content(content_bloc content)
    //ecriture
//write_on_tar(content_bloc *tab, int ouverture)
//write_on_file(content_bloc *tab, char *fileName)
//write_on_dir(content_bloc *tab, char *dirName)

#endif