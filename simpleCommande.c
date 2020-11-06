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
char args[50][50];
int i_args = 0;
const char space[2] = " ";
void fillArgs(char *commande){
    //fill by token
    char com[512];
    strcpy(com, commande);
    com[strlen(commande)] = '\0';
    char *read;
    read = strtok(com, space);
    if(read!=NULL){
        strcpy(args[i_args], read);
        args[i_args][strlen(read)] = '\0';
    }
    while( read != NULL ) {
        strcpy(args[i_args], read);
        args[i_args][strlen(read)] = '\0';
        i_args++;
        read = strtok(NULL, space);
    }
}
int adapter_cd(tsh_memory *memory){
    return cd(args[1], memory);
}
int adapter_pwd(tsh_memory *memory){
    return pwd(memory);
}
int adapter_mkdir(tsh_memory *memory){
    return mkdir(args[1], memory);
}
typedef int (*pt_adapter) (tsh_memory *memory);
pt_adapter listFun [NB_FUN] = { adapter_cd, adapter_pwd, adapter_mkdir};
int getFuncitonIndex(char *name){
    for(int i=0; i<NB_FUN; i++){
        if(strcmp(name, listCommande[i])==0)return i;
    }
    return -1;
}
int execSimpleCommande(char *commande, tsh_memory *memory){
    fillArgs(commande);
    (*(listFun[getFuncitonIndex(args[0])]))(memory);
    return 0;
}