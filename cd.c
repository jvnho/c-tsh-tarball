#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/errno.h>

#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"
#define BUFSIZE 512
char *concate_string(char *s1, char *s2);

int if_cd_is_valid(int descriptor, char * PATH, char * directory){
    lseek(descriptor, 0, SEEK_SET);

    struct posix_header *header = malloc(512);
    int nb_bloc_file = 0;
    char * recherched_path = concatString(PATH, directory);

    while(read(descriptor, header, 512)>0){//parcour de tete en tete jusqu' a la fin
        if(strcmp(header->name, recherched_path)==0)return 1;
        int tmp = 0;
        sscanf(header->size, "%o", &tmp);
        nb_bloc_file = (tmp + 512 -1) / 512;
        for(int i=0; i<nb_bloc_file; i++){
            read(descriptor, header, 512);
        }
    }
    return 0;
}

//directory is the argument given, PATH is the path from tsh_memory
int cd_in_tar(char * directory, char *PATH, char *tar_fd, char *tar_name){//modify the current path in the memory

    if(strcmp(".",directory)==0) return 0;
    //remove . in the directory
    if(strstr(directory, "..") == NULL) {//doesn't contains substring ".."

        int tar_descriptor = atoi(tar_fd);
        if(if_cd_is_valid(tar_descriptor, PATH, directory)){
            if(directory[strlen(directory)-1] == '/')//to check if we should add / at the end
                strcat(PATH, directory);//simple concat
            else
                strcat(PATH, concatString(directory, ""));//concat that add  / at the end
            return 0;
        }else{
            write(1, "no such directory\n", strlen("no such directory\n"));
            return -1;
        }
    }
    else{
        char *tar_name_plus_path = concate_string(tar_name, PATH); //concatenation : tar_name.tar/dir1/dir2/
        int index_last_slash = get_prev_directory(tar_name_plus_path); // gives index of first slash starting from the end ( check string_traitement.c for more info)
        if(index_last_slash == -1){ //exiting the tar -> erasing tsh_memory's data
            memset(PATH, 0, BUFSIZE);
            memset(tar_fd, 0, BUFSIZE);
            memset(tar_name, 0, BUFSIZE);
        } else {
            if(strncmp(tar_name_plus_path,tar_name, index_last_slash) == 0) //notice it's strNcmp not strcmp
                memset(PATH, 0, BUFSIZE); //user is now located in the root of the tar
            else
                PATH[index_last_slash-strlen(tar_name)+1] = '\0'; //reducing the PATH of one directory
        }
        return 0;
    }
    return 0;
}

int cd(char *directory, tsh_memory *memory){
    if(in_a_tar(memory)){//in a anormal circumstances
        return cd_in_tar(directory, memory->FAKE_PATH, memory->tar_descriptor, memory->tar_name);
    }

    // beforeTar/ directory.tar / afterTar
    char beforeTar[512]; char tarName[512]; char afterTar[512];
    //instanciate the format befor/ inside/ after (tar)
    getPreTar(directory, beforeTar);
    getTarName(directory, tarName);
    getPostTar(directory, afterTar);
    //if we have a pre Tar we apply chdir on that part
    if(strlen(beforeTar)){
        if(chdir(beforeTar)==-1){
            perror("");
            return -1;
        }
    }
    //if the is a directory.tar we instanciate the memory, and continue with the afterTar if it exists
    if(strlen(tarName)){
        if(instanciate_tsh_memory(tarName, memory)==-1) return -1;
        if(strlen(afterTar)) return cd_in_tar(afterTar, memory->FAKE_PATH, memory->tar_descriptor, memory->tar_name);
    }
    return 0;
}


char *concate_string(char *s1, char *s2){
    char *ret = malloc((strlen(s1)+strlen(s2)+1)*sizeof(char));
    sprintf(ret,"%s%s%c", s1, s2, '\0');
    return ret;
}
