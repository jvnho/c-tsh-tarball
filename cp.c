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
int temp_fd;
tsh_memory old_memory;
char location[512];
//cp somthing from tar, in a tar -> befor go to the tar collect all the bloc and the fd of the tar, the cd to target, if we are in tar getThe fd and, execut this function
int cp_tar_tar(char *source, char *target, int fd_source, int fd_target, char *fake_path){
    int nb_header = fill_fromTar(content, source, target, fd_source, fake_path, &i_content);
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
void simple_mkdir(char *directory, int fd_target){
    put_at_the_first_null(fd_target);
    struct posix_header *new_head = create_header(directory, 1, 0);
    write(fd_target, new_head, 512);
    writeZero(fd_target);
}
int cp_dir_tar(char *directory, char *target, int fd_target){
    
    char name_concat[512];
    char plus_slach[512];
    concatenation(target, directory, name_concat);
    concatenationPath(name_concat, "", plus_slach);
    if(dir_exist(fd_target, plus_slach))return 0;//alerady exist
    simple_mkdir(plus_slach, fd_target);
    DIR *dir = opendir(directory);
    struct dirent * inoeud_nom;
    while((inoeud_nom = readdir(dir))){
        
        if((strcmp(inoeud_nom->d_name, ".") != 0)&&(strcmp(inoeud_nom->d_name, "..") != 0)){
            struct stat buff;
            
            concatenationPath(directory, inoeud_nom->d_name, name_concat);
            
            if(lstat(name_concat, &buff)==-1)perror("lstat:");
            if(S_IFDIR & buff.st_mode){//if it's a dir
                cp_dir_tar(name_concat, target, fd_target);
            }else if(S_IFMT & buff.st_mode){//if it's a file
                cp_file_tar(name_concat, target, fd_target);
            }
        }
    }
    closedir(dir);
    return 0;
}
void createDir(content_bloc dirBloc){
    int pid_fils = fork();
    if(pid_fils){//parent
        waitpid(pid_fils, NULL, 0);
    }else{//child
        execlp("mkdir", "mkdir", dirBloc.hd.name, NULL);
    }
}
void createFile(content_bloc fileBloc){
    if((temp_fd = open(fileBloc.hd.name, O_WRONLY|O_CREAT| O_APPEND, 0664))== -1){//create file
        write(2, "cp :failed to extract file\n", strlen("cp :failed to extract file\n"));
    }else{//then write it's content
        for(int i = 0; i<fileBloc.nb_bloc; i++){
            write(temp_fd, fileBloc.content[i], strlen(fileBloc.content[i]));
        } 
        close(temp_fd);
    }
}
//dir or file form tar to outside
int cp_tar_outside(char *file, int fd_source, char *fake_path){
    int nb_header = fill_fromTar(content, file, "", fd_source, fake_path, &i_content);
    for(int i = 0; i<nb_header; i++){
        
        if(content[i].hd.typeflag == '5'){//Dossier
            createDir(content[i]);
        }else{//fichier
            createFile(content[i]);
        }
    }
    return 0;
}
//for one argument
int copy(char listOption[50][50], char *source, char *target, tsh_memory *memory){
    copyMemory(memory, &old_memory);
    getLocation(source, location);
    int lenLocation = strlen(location);
    char *fileToCopy = source;
    if(lenLocation){//if there is an extra path, cd to that path
        if(cd(location, memory)==-1){
            return -1;
        }
        fileToCopy = source + lenLocation;
    }
    //cd vers le target pour juste obtenir le nom et fd
    //après cd vers source, comme ca on peut fill form avec le nom target
    //après cd vers target pour écrire ce qu'on a recu dans le tableau
    return 0;
}
int main(int n, char **args){
    //source target .tar fakePaht
    int fd_tar = open(args[1], O_RDWR);
    //doss1
    int tail = cp_tar_outside("doss2/Clause.java", fd_tar, "");
    
    
    return 0;
}