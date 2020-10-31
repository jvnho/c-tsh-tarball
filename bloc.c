#include <unistd.h>
#include "bloc.h"
#include "string_traitement.h"
char FILE_PATH[512];
//get all header_name and content bloc if the header name is (the source/ or source/X or source)
int fill_fromTar(content_bloc *tab, char *source, char *target, int descriptor, char *fake_path){//target should have a '/' at the end
    
    char *path_to_source = concatString(fake_path, source);
    lseek(descriptor, 0, SEEK_SET);
    struct posix_header header;
    int nb_bloc_file = 0;
    int index = 0;
    char new_name[512];//name to write on the new header
    while(read(descriptor, &header, 512)>0){//parcour de tete en tete jusqu' a la fin
        strncpy(FILE_PATH, header.name, strlen(path_to_source));
        if(strcmp(FILE_PATH, path_to_source) == 0){//found a bloc to cp
            tab[0].hd = copyHeader(header, concatString(target, strcpy(new_name, header.name + strlen(fake_path))));
            /*
                copie a l'indice index le header et les bloc
            */
        }else{//not a interesting bloc so we just skip it using lseek
            int tmp = 0;
            sscanf(header.size, "%o", &tmp);
            nb_bloc_file = (tmp + 512 -1) / 512;
            lseek(descriptor, nb_bloc_file * 512, SEEK_CUR);
        }
        
    }
    return 0;
}

