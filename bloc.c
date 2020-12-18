#include <unistd.h>
#include <sys/errno.h>
#include <fcntl.h>
#include "bloc.h"
#include "string_traitement.h"
char FILE_PATH[512];
int getIndexLastSlach(char *path){
    int n = strlen(path) - 2;
    for(int i = n; 0<=i; i--){
        if(path[i] == '/')return i;
    }
    return -1;
}
//get all header_name and content bloc if the header name is fake/source (file)  or fake/source/X  (dierctory)
int fill_fromTar(content_bloc *tab, char *source, char *target, int descriptor, char *fake_path){
    //target should have a '/' at the end
    //source should have a '/' at the end if it's a directory
    char *path_to_source = simpleConcat(fake_path, source);//verification slach si dossier j'appel concat string, si fichier j'appel simple concat
    lseek(descriptor, 0, SEEK_SET);
    struct posix_header header;
    int tmp = 0;
    int nb_bloc_file = 0;
    int index_tab = 0;
    int index_content = 0;
    char new_name[512];//name to write on the new header
    int result_read = 0;
    int without_path = getIndexLastSlach(path_to_source) + 1;//used to remove the path 
    while((result_read = read(descriptor, &header, 512))>0){//parcour de tete en tete jusqu' a la fin
    
        strncpy(FILE_PATH, header.name, strlen(path_to_source));
        
        if(strcmp(FILE_PATH, path_to_source) == 0){//found a bloc to cp
            //fill the the header 
            tab[index_tab].hd = copyHeader(header, simpleConcat(target, strcpy(new_name, header.name + without_path)));
            
            //fill the bloc
            sscanf(header.size, "%o", &tmp);
            nb_bloc_file = (tmp + 512 -1) / 512;
            for(int i=0; i<nb_bloc_file; i++){
                read(descriptor, tab[index_tab].content[index_content], 512);
                index_content++;
            }
            tab[index_tab].nb_bloc = index_content;
            index_tab++; 
            index_content = 0;  
        }else{//not a interesting bloc so we just skip it using lseek
            sscanf(header.size, "%o", &tmp);
            nb_bloc_file = (tmp + 512 -1) / 512;
            lseek(descriptor, nb_bloc_file * 512, SEEK_CUR);
        }  
    }
    if(result_read == -1){
        perror("");
        return -1;
    }
    return index_tab;
}
int fill_fromFile_outside(content_bloc *tab, char *source, char *target, int* starting_index){
    int fd_file;
    if((fd_file = open(source, O_RDONLY)) == -1){
        perror("");
        return -1;
    }
    //fill the bloc
    int nb_bloc = 0;
    int leng = 0;
    int i = 1;
    int sizeFile = 0;
    while((leng = read(fd_file, tab[*starting_index].content[nb_bloc], 512))>0){
        if(leng<512){//complet the rest of this bloc by zero
            char end_bloc[512-leng];
            memset(end_bloc, 0, 512-leng);
            tab[*starting_index].content[nb_bloc][leng] = '\0';
            strcat(tab[*starting_index].content[nb_bloc], end_bloc);
            sizeFile = sizeFile + leng;
            nb_bloc++;
            break;
        }
        sizeFile = sizeFile + leng;
        nb_bloc++;
        i++;
    }
    
    if(leng == -1){//error from read
        perror("");
        close(fd_file);
        return -1;
    }
    struct posix_header * newHead = create_header(concate_string(target, source), 0, sizeFile);
    tab[*starting_index].hd = *newHead;
    tab[*starting_index].nb_bloc = nb_bloc;
    close(fd_file);
    *starting_index = *(starting_index) + 1;
    return 0;
}

