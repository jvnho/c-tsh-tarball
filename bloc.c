#include <unistd.h>
#include "bloc.h"
#include "tar.h"
char FILE_PATH[512];
//get all header_name and content bloc if the header name is (the source/ or source/X or source)
int fill_fromTar(content_bloc *tab, char *source, int descriptor){
    int index = 0;
    lseek(descriptor, 0, SEEK_SET);
    struct posix_header header;
    int nb_bloc_file = 0;
    
    while(read(descriptor, &header, 512)>0){//parcour de tete en tete jusqu' a la fin
        strncpy(FILE_PATH, header.name, strlen(source));
        if(strcmp(FILE_PATH, source) == 0){//found a bloc to cp
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

