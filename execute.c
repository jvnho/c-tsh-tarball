#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "tsh_memory.h"
#include "simpleCommande.h"
#include "pipe.h"
int pipe_tsh(tsh_memory *m, tsh_memory *a);
int execute(tsh_memory *memory){
    if(strstr(memory->comand, "|")==NULL){//Pas de pipe
        
    }else{
        write(1, "avec pipe\n", strlen("avec pipe\n"));
    }
    //pipe(memory, memory);
    return 0;
}