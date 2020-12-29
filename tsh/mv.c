#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#include "tsh_memory.h"
#include "cp.h"
#include "cd.h"
#include "rm.h"
#include "string_traitement.h"
#include "exec_funcs.h" //exec_funcs.h
#include "tar.h"

int do_mv(tsh_memory *memory, char *src, char *target,char option[50][50], int nb_opt){
    tsh_memory tmp;
    int arg_r = 0;
    if(src[strlen(src)-1] == '/' || is_unix_directory(src) == 1){ 
        arg_r = 1;//we are sure user entered a directory
    }   
    else 
    {
        copyMemory(memory,&tmp);

        char location[512];
        char *without_path = src;
        getLocation(src, location);
        if(strlen(location) > 0){
            if(cd(location, &tmp) == -1) return -1;
            without_path += strlen(location);
        }
        if(in_a_tar(&tmp) == 0)
        {
            DIR *directory;
            if((directory = opendir(without_path))){
                arg_r = 1; //dir exists that means user wants cp a directory
                closedir(directory);
            }
        } else {
            char src_fullpath[512];
            concatenationPath(tmp.FAKE_PATH, without_path, src_fullpath); //adds slash to the concatenation to make it appears like a directory in the tarball
            if(dir_exist(atoi(tmp.tar_descriptor), src_fullpath) == 1) 
                arg_r = 1;
        }
    }
    if(copy(option, nb_opt, src, target, memory, arg_r) == -1){
        return -1;
    }
    do_rm(memory, src, option, nb_opt, arg_r);
    return 1;
}

int mv(tsh_memory *memory, char args[50][50], int nb_args, char option[50][50], int nb_opt){
    if(nb_args < 2)
    {
        char *err = "mv: opérande de fichier manquant";
        write(2, err, strlen(err));
        return -1;
    }
    char *target = args[nb_args-1];
    for(int i = 0; i < nb_args-1; i++)
    {
        do_mv(memory, args[i], target, option,nb_opt);
    }
    return 1;
}