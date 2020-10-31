#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "bloc.h"
#include "tar.h"
content_bloc content[512];
int cp_tar_tar(char *source, char *target, int fd_tar, char *fake_path){
    int content_length = fill_fromTar(content, source, target, fd_tar, fake_path);
    return content_length;
}
int main(int n, char **args){
    //source target .tar fakePaht
    int fd_tar = open(args[3], O_RDONLY);
    int tail = cp_tar_tar(args[1], args[2], fd_tar, "");
    
    for(int i = 0; i<tail; i++){
        printf("-------name = %s\n", content[i].hd.name);
        for(int j = 0; j<content[i].nb_bloc; j++){
            printf("\n");
            printf("%s", content[i].content[j]);
            printf("\n");
        }
    }
    return 0;
}