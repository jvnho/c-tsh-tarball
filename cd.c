#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "tar.h"
char * concatString(char * path, char *dir){
    int tail = strlen(path)+strlen(dir)+1;
    char * result = malloc(tail);
    strcpy(result, path);
    strcat(result, dir);
    result[tail-1] = '\0';
    return result;
}
void getIntoDirectory(int descripteur, char * PATH, char * directory){
    lseek(descripteur, 0, SEEK_SET);
    struct posix_header *tete = malloc(512);
    int nb_bloc_fichier = 0;
    while(read(descripteur, tete, 512)>0){//parcour de tete en tete jusqu' a la fin


        int tmp = 0;
        sscanf(tete->size, "%o", &tmp);
        nb_bloc_fichier = (tmp + 512 -1) / 512;
        for(int i=0; i<nb_bloc_fichier; i++){
            read(descripteur, tete, 512);
        }
    }
}
void cd(int descripteur, char * PATH, char * directory){

}
int main(void){
    char *test = concatString("Path/nom/", "directory");
    printf("%s\n", test);
    return 0;
}