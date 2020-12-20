#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <assert.h>
#include "bloc.h"
#include "tar.h"
#include "string_traitement.h"
#include "cd.h"
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
int cp_tar_outside(char *file, char *target, int fd_source, char *fake_path){
    int nb_header = fill_fromTar(content, file, target, fd_source, fake_path, &i_content);
    for(int i = 0; i<nb_header; i++){
        
        if(content[i].hd.typeflag == '5'){//Dossier
            createDir(content[i]);
        }else{//fichier
            createFile(content[i]);
        }
    }
    return 0;
}
void restoreMemory(tsh_memory *old_memory, tsh_memory *memory){
    copyMemory(old_memory, memory);
    char *destination = malloc(strlen(memory->REAL_PATH));
    strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);//remove the $
    cd(destination, memory);
}
//when we restore the cd, it wil close the descriptor, so save it using dup
void saveDescirptor(tsh_memory *memory){
    int fd = atoi(memory->tar_descriptor);
    fd = dup(fd);
    memset(memory->tar_descriptor, 0, 512);
    strcpy(memory->tar_descriptor, int_to_string(fd));
}
char *removeSlach(char *dir){
    if(dir[strlen(dir)-1] == '/')dir[strlen(dir)-1] = '\0';
    return dir;
}
void addSlach(char *dir, char *result){
    memset(result, 0, 512);
    strcpy(result, dir);
    if(dir[strlen(dir)-1] != '/')
        concatenationPath(dir, "", result);
    
}
char **fusionCommand(char option[50][50], int size_option, char *source, char *target){
    char **result;
    assert((result = malloc((size_option + 4)*sizeof(char *))) !=NULL);
    assert((result[0] = malloc((strlen("cp")+1) *sizeof(char))) != NULL);
    strcpy(result[0], "cp");
    result[0][2] = '\0';
    int index_result = 1;
    for(int i = 0; i< size_option; i++){
        assert((result[index_result] = malloc(strlen(option[i])*sizeof(char))) != NULL);
        strcpy(result[index_result], option[i]);
        index_result++;
    }
    result[index_result] = source;
    result[index_result + 1] = target;
    result[index_result + 2] = NULL;
    return result;
}
void exec_cp(char option[50][50], int size_option, char *source, char *target){
    char **args = fusionCommand(option, size_option, source, target);
    execvp(args[0], args);
}
//for one argument
int copy(char listOption[50][50], int size_option, char *source, char *real_target, tsh_memory *memory){
    char target[512];
    addSlach(real_target, target);
    copyMemory(memory, &old_memory);
    if(strlen(target)){
        if(cd(target, memory)==-1)return -1;
    }
    //save the state of target befor restor cd
    tsh_memory memoryTarget;
    copyMemory(memory, &memoryTarget);
    saveDescirptor(&memoryTarget);
    //retore the initial state
    restoreMemory(&old_memory, memory);
    //cd to source
    char location[512];//the path inside the source before geting to the file to copy
    getLocation(source, location);
    int lenLocation = strlen(location);
    char *fileToCopy = source;
    if(lenLocation){//there is an extraPath to cd
        if(cd(location, memory)==-1)return -1;
        fileToCopy = source + lenLocation;
    }
    int returnValue = 0;
    //from ?? to -> .tar
    if(in_a_tar(&memoryTarget)){ 
        //from .tar to -> .tar
        if(in_a_tar(memory)){
            returnValue = cp_tar_tar(fileToCopy, memoryTarget.FAKE_PATH, atoi(memory->tar_descriptor), 
            atoi(memoryTarget.tar_descriptor), memory->FAKE_PATH);
            restoreMemory(&old_memory, memory);
            return returnValue;
        }//from outside to -> .tar
        else{
            //check the file type
            struct stat status;
            if(lstat(fileToCopy, &status)==-1){//case of error restore the memory
                perror("cp: ");
                restoreMemory(&old_memory, memory);
                return -1;
            }
            //directory
            if(S_IFDIR & status.st_mode){//attention si il y a un slach a la fin
                returnValue = cp_dir_tar(removeSlach(fileToCopy), memoryTarget.FAKE_PATH, atoi(memoryTarget.tar_descriptor));
                restoreMemory(&old_memory, memory);
                return returnValue;
            }
            //file
            returnValue = cp_file_tar(fileToCopy, memoryTarget.FAKE_PATH, atoi(memoryTarget.tar_descriptor));
            restoreMemory(&old_memory, memory);
            return returnValue;
        }
    }//form ?? to -> outside
    else{
        //tar -> outside
        if(in_a_tar(memory)){
            returnValue = cp_tar_outside(fileToCopy, target, atoi(memory->tar_descriptor), memory->FAKE_PATH);
            restoreMemory(&old_memory, memory);
            return returnValue;
        }//outside -> outside
        else{
            restoreMemory(&old_memory, memory);
            int pid_fils = fork();
            if(pid_fils){//parent
                int status;
                waitpid(pid_fils, &status, WUNTRACED);
                if(WEXITSTATUS(status)==-1)return -1;
            }else exec_cp(listOption, size_option, source, target);
        }
    }
    return 0;
}
int main(int n, char **args){
    //source target .tar fakePaht
    int fd_tar = open(args[1], O_RDWR);
    //doss1
    int tail = cp_tar_outside("doss2/", "d1/d2/",fd_tar, "");
    
    
    return 0;
}