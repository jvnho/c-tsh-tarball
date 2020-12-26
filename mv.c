#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#include "tsh_memory.h"
#include "cp.h"
#include "cd.h"
#include "rm.h"
#include "string_traitement.h"
#include "function.h" //exec_funcs.h
#include "tar.h"


int mv(tsh_memory *memory, char args[50][50], int nb_args, char option[50][50], int nb_opt){
    for(int i = 0; i < nb_args-1; i++)
    {
        int arg_r = 0;
        if(args[i][strlen(args[i])-1] == '/' || is_unix_directory(args[i]) == 1)
        { 
            arg_r = 1;//we are sure user entered a directory
        } 
        else 
        {
            tsh_memory tmp;
            copyMemory(memory,&tmp);

            char location[512];
            memset(location, 0, sizeof(location));

            char *arg_without_path = args[i];
            getLocation(args[i], location);
            if(strlen(location) > 0){
                if(cd(location, &tmp) == -1) return -1;
                arg_without_path += strlen(location);
            }
            if(in_a_tar(&tmp) == 0){
                DIR *directory;
                if((directory = opendir(arg_without_path)) != NULL){
                    arg_r = 1; //dir exists that means user wants cp a directory
                    closedir(directory);
                }
            } else {
                char *src_fullpath = concatDirToPath(tmp.FAKE_PATH, arg_without_path); //adds slash to the concatenation to make it appears like a directory in the tarball
                if(dir_exist(atoi(tmp.tar_descriptor), src_fullpath) == 1) arg_r = 1;
            }
        }
        if(copy(option, nb_opt, args[i], args[nb_args-1],memory, arg_r) == -1) continue;
        do_rm(memory, args[i], option, nb_opt, arg_r);
    }
    return 1;
}