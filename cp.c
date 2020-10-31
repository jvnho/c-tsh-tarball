#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "bloc.h"
#include "tar.h"
content_bloc content[512];
//cp somthing from tar, in a tar
int cp_tar_tar(char *source, char *target, int fd_source, int fd_target, char *fake_path){
    int content_length = fill_fromTar(content, source, target, fd_source, fake_path);
    put_at_the_first_null(fd_target);
    //write all the bloc in tab and the last bloc null
    return content_length;
}
int main(int n, char **args){
    //source target .tar fakePaht
    int fd_tar = open(args[3], O_RDONLY);
    int tail = cp_tar_tar(args[1], args[2], fd_tar, fd_tar,"");
    
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