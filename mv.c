#include <string.h>

#include "tsh_memory.h"
#include "cp.h"
#include "rm.h"

int mv(tsh_memory *mem, char args[50][50], int nb_args, char option[50][50], int nb_opt){
    //strcpy(option[nb_args], "-r"); //adding -r to copy/delete dir if given
    if(nb_opt){
         if(copy_tar(option, args, nb_opt, nb_args, mem) == -1) return -1;
    } else { 
        if(copy_tar(NULL, args, nb_opt, nb_args, mem) == -1) return -1;
    }
    if(rm(mem, args, nb_args-1, option, nb_opt) == -1) return -1; //(nb_args-1) because the last arg given by user is the target
    return 1;
}