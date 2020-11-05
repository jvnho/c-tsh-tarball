#include <string.h>
#include <stdio.h>
#include "tsh_memory.h"
#include "cd.h"
#include "pwd.h"
#include "mkdir.h"
#include "ls.h"
#include "tsh_memory.h"
#include "string_traitement.h"
char *listCommande[] = {"cd", "pwd", "mkdir"};
#define NB_FUN 3
int getFuncitonIndex(char *name){
    for(int i=0; i<NB_FUN; i++){
        if(strcmp(name, listCommande[i])==0)return i;
    }
    return -1;
}
int adapter_cd(tsh_memory *memory, char **args){
    return cd(args[0], memory);
}
int adapter_pwd(tsh_memory *memory, char **args){
    return pwd(memory);
}
int adapter_mkdir(tsh_memory *memory, char **args){
    return mkdir(args[0], memory);
}
typedef int (*pt_adapter) (tsh_memory *memory, char **args);
pt_adapter listFun [NB_FUN] = { adapter_cd, adapter_pwd, adapter_mkdir};
int execSimpleCommande(char *commande, tsh_memory *memory){
    char space[2] = " ";
    
    return 0;
}