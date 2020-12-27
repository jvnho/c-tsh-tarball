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

int mv(tsh_memory *memory, char args[50][50], int nb_args, char option[50][50], int nb_opt){
    if(nb_args < 2){
        char *err = "mv: opérande de fichier manquant";
        write(2, err, strlen(err));
        return -1;
    }
    tsh_memory tmp;
    for(int i = 0; i < nb_args-1; i++)
    {
        int arg_r = 0;
        if(args[i][strlen(args[i])-1] == '/' || is_unix_directory(args[i]) == 1){ 
            arg_r = 1;//we are sure user entered a directory
        }   
        else 
        {
            copyMemory(memory,&tmp);

            char location[512];
            char *without_path = args[i];
            getLocation(args[i], location);
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
                char *src_fullpath = concatDirToPath(tmp.FAKE_PATH, without_path); //adds slash to the concatenation to make it appears like a directory in the tarball
                if(dir_exist(atoi(tmp.tar_descriptor), src_fullpath) == 1) arg_r = 1;
            }
        }
        if(copy(option, nb_opt, args[i], args[nb_args-1],memory, arg_r) == -1){
            continue;
        }
        do_rm(memory, args[i], option, nb_opt, arg_r);
    }
    return 1;
}