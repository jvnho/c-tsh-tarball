#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include "bloc.h"
#include "tar.h"
#include "string_traitement.h"
content_bloc content[512];//fill it befor the specifc call of cp (cp_tar_tar or cp_tar_dir)
int i_content = 0;//don't forget to reset this, with the tab
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
int cp_file_tar(char *source, char *target, int fd_target){
    if(fill_fromFile_outside(content, source, target, &i_content))return -1;//fill the tab
    put_at_the_first_null(fd_target);//positioning at the first null
    if(write(fd_target, &(content[i_content - 1].hd), 512) == -1){//write the head 
        perror("");
        return -1;
    }
    int nb_write = 1;
    for(int iBloc = 0; iBloc < content[i_content - 1].nb_bloc; iBloc++){//write the bloc
        if(write(fd_target, content[i_content - 1].content[0], 512) == -1){
            perror("");
            return -1;
        }
        nb_write++;
    }
    //write the end of bloc
    if(nb_write == 1){
        return writeZero(fd_target);
    }if(nb_write > 1){
        if(writeZero(fd_target)==-1)return -1;
        return writeZero(fd_target);
    }
    return 0;
}
int cp_dir_tar(char *directory, char *target, int fd_target){
    
    char name_concat[512];
    char plus_slach[512];
    concatenation(target, directory, name_concat);
    concatenation(name_concat, "", plus_slach);
    printf("ce qu'on veu creer = %s\n", plus_slach);
    DIR *dir = opendir(directory);
    struct dirent * inoeud_nom;
    while((inoeud_nom = readdir(dir))){
        
        if((strcmp(inoeud_nom->d_name, ".") != 0)&&(strcmp(inoeud_nom->d_name, "..") != 0)){
            struct stat buff;
            
            concatenation(directory, inoeud_nom->d_name, name_concat);
            if(lstat(name_concat, &buff)==-1)perror("lstat:");
            if(S_IFDIR & buff.st_mode){//if it's a dir
                cp_dir_tar(name_concat, target, fd_target);
            }else if(S_IFMT & buff.st_mode){//if it's a file
                printf("creation de = %s\n", name_concat);
                //cp_file_tar(name_concat, target, fd_target);
            }
        }
    }
    closedir(dir);
    return 0;
}

int main(int n, char **args){
    //source target .tar fakePaht
    int fd_tar = open(args[3], O_RDWR);
    int tail = cp_dir_tar(args[1], args[2], fd_tar);
    
    
    return 0;
}