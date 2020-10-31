#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "bloc.h"
#include "tar.h"
content_bloc content[512];//fill it befor the specifc call of cp (cp_tar_tar or cp_tar_dir)
int i_content = 0;
//cp somthing from tar, in a tar -> befor go to the tar collect all the bloc and the fd of the tar, the cd to target, if we are in tar getThe fd and, execut this function
int cp_tar_tar(char *source, char *target, int fd_source, int fd_target, char *fake_path){
    int nb_header = fill_fromTar(content, source, target, fd_source, fake_path);
    if(nb_header == -1)return -1;
    put_at_the_first_null(fd_target);
    //write all the bloc in tab and the last bloc null
    int nb_write = 0;
    for(int iHeader = 0; iHeader< nb_header; iHeader++){
        if(write(fd_target, &(content[iHeader].hd), 512) == -1){//write the header
            perror("");
            return -1;
        }
        nb_write++;
        for(int iBloc = 0; iBloc < content[iHeader].nb_bloc; iBloc++){//write the bloc
            if(write(fd_target, content[iHeader].content[iBloc], 512) == -1){
                perror("");
                return -1;
            }
            nb_write++;
        }
    }
    //write the end of bloc
    if(nb_write == 1){
        return writeZero(fd_target);
    }if(nb_write >= 1){
        if(writeZero(fd_target)==-1)return -1;
        return writeZero(fd_target);
    }
    return 0;
}
int main(int n, char **args){
    //source target .tar fakePaht
    int fd_tar = open(args[3], O_RDWR);
    int tail = cp_tar_tar(args[1], args[2], fd_tar, fd_tar,"");
    
    
    return 0;
}