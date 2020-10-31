#include <unistd.h>
#include "bloc.h"
#include "string_traitement.h"
char FILE_PATH[512];
//get all header_name and content bloc if the header name is (the source/ or source/X or source)
int fill_fromTar(content_bloc *tab, char *source, char *target, int descriptor, char *fake_path){//target should have a '/' at the end
    
    char *path_to_source = concatString(fake_path, source);
    
    lseek(descriptor, 0, SEEK_SET);
    struct posix_header header;
    int tmp = 0;
    int nb_bloc_file = 0;
    int index_tab = 0;
    int index_content = 0;
    char new_name[512];//name to write on the new header
    
    while(read(descriptor, &header, 512)>0){//parcour de tete en tete jusqu' a la fin
    
        strncpy(FILE_PATH, header.name, strlen(path_to_source));
        
        
        if(strcmp(FILE_PATH, path_to_source) == 0){//found a bloc to cp
            //fill the the header
            printf("trouver = %s\n", header.name);
            tab[index_tab].hd = copyHeader(header, simpleConcat(target, strcpy(new_name, header.name + strlen(fake_path))));
            //fill the bloc
            /*
            sscanf(header.size, "%o", &tmp);
            nb_bloc_file = (tmp + 512 -1) / 512;
            for(int i=0; i<nb_bloc_file; i++){
                read(descriptor, tab[index_tab].content[index_content], 512);
                index_content++;
            }
            tab[index_tab].nb_bloc = index_content;
            index_tab++; 
            index_content = 0;  */
            sscanf(header.size, "%o", &tmp);
            nb_bloc_file = (tmp + 512 -1) / 512;
            lseek(descriptor, nb_bloc_file * 512, SEEK_CUR);
        }else{//not a interesting bloc so we just skip it using lseek
            sscanf(header.size, "%o", &tmp);
            nb_bloc_file = (tmp + 512 -1) / 512;
            lseek(descriptor, nb_bloc_file * 512, SEEK_CUR);
        } 
        
    }
    //return index_tab;
    return 0;
}

