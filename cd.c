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
int shouldSave = 1;

char *concate_string(char *s1, char *s2);

int if_cd_is_valid(int descriptor, char * PATH, char * directory){
    char * recherched_path = concatString(PATH, directory);
    return dir_exist(descriptor, recherched_path);
}
void reduceFakePath(char * directory, tsh_memory *mem){
    char *tar_name_plus_path = concate_string(mem->tar_name, mem->FAKE_PATH); //concatenation : tar_name.tar/dir1/dir2/
        int index_last_slash = get_prev_directory(tar_name_plus_path); // gives index of first slash starting from the end ( check string_traitement.c for more info)
        if(index_last_slash == -1){ //exiting the tar -> erasing tsh_memory's data
            clearMemory(mem);
        } else {
            if(strncmp(tar_name_plus_path,mem->tar_name, index_last_slash) == 0) //notice it's strNcmp not strcmp
                clearFakePath(mem); //user is now located in the root of the tar
                //switch
            else
                mem->FAKE_PATH[index_last_slash-strlen(mem->tar_name)+1] = '\0'; //reducing the PATH of one directory
        }
}
int cd(char *directory, tsh_memory *memory);

//directory is the argument given, PATH is the path from tsh_memory
int cd_in_tar(char * directory, tsh_memory *memory){//modify the current path in the memory
    //char *PATH, char *tar_fd, char *tar_name
    if(strcmp(directory,".") == 0){ //ne marche pas encore, ça ne détecte pas le point sur l'entrée  standard
        return 0;
    }
    if(strstr(directory, "..") == NULL) {//doesn't contains substring ".."

        int tar_descriptor = atoi(memory->tar_descriptor);
        if(if_cd_is_valid(tar_descriptor, memory->FAKE_PATH, directory)){
            if(directory[strlen(directory)-1] == '/')//to check if we should add / at the end
                strcat(memory->FAKE_PATH, directory);//simple concat
            else
                strcat(memory->FAKE_PATH, concatString(directory, ""));//concat that add  / at the end
            return 0;
        }else{
            write(1, "Noo such directory\n", strlen("Noo such directory\n"));
            return -1;
        }
    }
    else{
        if(directory[0] == '.'){//we are starting by ../
            reduceFakePath(directory, memory);//we have done the first ../
            if(strlen(directory)>3){// ../somethig
                if(strlen(memory->tar_descriptor) == 0){//check if the first .. doesn't get us out of the tar
                    shouldSave = 0;
                    return cd(directory + 3, memory);
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
    if(shouldSave){
        saveMemory(memory, &save);
    }
    if(in_a_tar(memory) && directory[0] != '/'){//in a anormal circumstances and when it's not an absolute path
        remove_simple_dot_from_dir(directory); // remove eventual ./, /./, /. from the directory (details: string_traitement.c)
        if(cd_in_tar(directory, memory)==-1){
            saveMemory(&save, memory);//restore
            shouldSave = 1;
            chdir(memory->REAL_PATH);
            return -1;
        }
        else if(in_a_tar(memory)==0){
            close(string_to_int(save.tar_descriptor));
        }
        return 0;
    }
    if(directory[0] == '/') clearMemory(memory);

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
            saveMemory(&save, memory);//error
            shouldSave = 1;
            chdir(memory->REAL_PATH);
            return -1;
        }
    }
    //if the is a directory.tar we instanciate the memory, and continue with the afterTar if it exists
    if(strlen(tarName)){
        if(instanciate_tsh_memory(tarName, memory)==-1) return -1;//should avoid the normal cd done before
        if(strlen(afterTar)){
            if(cd_in_tar(afterTar, memory) == -1){//if error we should't have done the first part
                saveMemory(&save, memory);
                memory->REAL_PATH[strlen(memory->REAL_PATH)-2] = '\0';
                chdir(memory->REAL_PATH);
                shouldSave = 1;
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
