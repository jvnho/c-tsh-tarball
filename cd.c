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
char firstDir[BUFSIZE];
tsh_memory save;//save so we can retore in case of error

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
void reduceFakePath(char * directory, char *PATH, char *tar_fd, char *tar_name){
    char *tar_name_plus_path = concate_string(tar_name, PATH); //concatenation : tar_name.tar/dir1/dir2/
        int index_last_slash = get_prev_directory(tar_name_plus_path); // gives index of first slash starting from the end ( check string_traitement.c for more info)
        if(index_last_slash == -1){ //exiting the tar -> erasing tsh_memory's data
            memset(PATH, 0, BUFSIZE);
            memset(tar_fd, 0, BUFSIZE);
            memset(tar_name, 0, BUFSIZE);
        } else {
            if(strncmp(tar_name_plus_path,tar_name, index_last_slash) == 0) //notice it's strNcmp not strcmp
                memset(PATH, 0, BUFSIZE); //user is now located in the root of the tar
                //switch
            else
                PATH[index_last_slash-strlen(tar_name)+1] = '\0'; //reducing the PATH of one directory
        }
}
int cd(char *directory, tsh_memory *memory);

//directory is the argument given, PATH is the path from tsh_memory
int cd_in_tar(char * directory, tsh_memory *memory){//modify the current path in the memory
    //char *PATH, char *tar_fd, char *tar_name
    if(strcmp(".",directory)==0) return 0;
    //remove . in the directory
    if(strstr(directory, "..") == NULL) {//doesn't contains substring ".."

        int tar_descriptor = atoi(memory->tar_descriptor);
        if(if_cd_is_valid(tar_descriptor, memory->FAKE_PATH, directory)){
            if(directory[strlen(directory)-1] == '/')//to check if we should add / at the end
                strcat(memory->FAKE_PATH, directory);//simple concat
            else
                strcat(memory->FAKE_PATH, concatString(directory, ""));//concat that add  / at the end
            return 0;
        }else{
            write(1, "no such directory\n", strlen("no such directory\n"));
            return -1;
        }
    }
    else{
        if(directory[0] == '.'){//we are starting by ../
            reduceFakePath(directory, memory->FAKE_PATH, memory->tar_descriptor, memory->tar_name);//we have done the first ../
            if(strlen(directory)>3){// ../somethig
                if(strlen(memory->tar_descriptor) == 0){//check if the fist .. doesn't get us out of the tar
                    printf("Sortiiiii\n");
                    return cd(directory + 3, memory);//save valeur corrompu
                }
                return cd_in_tar(directory+3, memory);
            }
        }else{
            //  doss/  ..  /doss2
            int index_first_slach = getFirstDir(directory, firstDir);
            int success = cd_in_tar(firstDir, memory);
            if(success == - 1)return -1;//first step
            return cd_in_tar(directory + index_first_slach + 1, memory);
        }
    }
    return 0;
}

int cd(char *directory, tsh_memory *memory){
    saveMemory(memory, &save);
    if(in_a_tar(memory)){//in a anormal circumstances
        if(cd_in_tar(directory, memory)==-1){
            saveMemory(&save, memory);
            return -1;
        }
        //if we did well we should check if we get out the tar so we need to close the descriptor
        return 0;
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
            printf("befor tar = %s\n", beforeTar);
            printf("tar_name = %s\n", save.tar_name);
            saveMemory(&save, memory);//error
            return -1;
        }
    }
    //if the is a directory.tar we instanciate the memory, and continue with the afterTar if it exists
    if(strlen(tarName)){
        if(instanciate_tsh_memory(tarName, memory)==-1) return -1;//should avoid the normal cd done before
        if(strlen(afterTar)){
            if(cd_in_tar(afterTar, memory) == -1){//if error we should't have done the first part
                saveMemory(&save, memory);
                return -1;
            }
            return 0;
        } 
    }
    return 0;
}


char *concate_string(char *s1, char *s2){
    char *ret = malloc((strlen(s1)+strlen(s2)+1)*sizeof(char));
    sprintf(ret,"%s%s%c", s1, s2, '\0');
    return ret;
}
//create fun comback chdir
