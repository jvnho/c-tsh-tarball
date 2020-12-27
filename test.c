#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

int main(void){
    DIR* dir = opendir("caca");
    if(dir == NULL){
        printf("pas ouvert\n");
    } else {
        printf("ouvert\n");
        closedir(dir);
    }
}