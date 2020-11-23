#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include "tsh_memory.h"
#include "cd.h"
#include "pwd.h"
#include "mkdir.h"
#include "ls.h"
#include "rmdir.h"
#include "tsh_memory.h"
#include "string_traitement.h"
char *listCommande[] = {"exit", "cd", "pwd", "mkdir", "ls", "rmdir"};
#define NB_FUN 6
char co[50];
char option[50][50];
int i_option = 0;
char args[50][50];
int i_args = 0;
const char space[2] = " ";
int returnval;
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
void resetArgs(){
    for(int i = 0; i<i_args; i++){
        memset(args[i], 0, 50);
    }
    i_args = 0;
}
void resetOptions(){
    for(int i= 0; i<i_option; i++){
        memset(option[i], 0, 50);
    }
    i_option = 0;
}
void resetCommand(){//reste every things
    memset(co, 0, 50);
    resetOptions();
    resetArgs();
}
//we can't pass just args throug exec because there is not null at the end, and we can't write arg[i]= NULL, because arg is unmodifiable, so no choice Malloc :/
char ** argsPlusNULL(){
    char **result;
    assert(result = malloc((i_args + 1) * sizeof(char *)));//we can't pass just args
    for(int i = 0; i<i_args; i++){
        assert(result[i] = malloc(strlen(args[i])*sizeof(char)));
        strcpy(result[i], args[i]);
    }
    result[i_args] = NULL;
    return result;
}
//Adapter Pattern
int adapter_exit(tsh_memory *memory){
    return exit2(memory);
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
int adapter_ls(tsh_memory *memory){
    return ls(memory);
}
int adapter_rmdir(tsh_memory *memory){
    return rmdir_func(memory, args[1]);
}
typedef int (*pt_adapter) (tsh_memory *memory);//declaration pointer of function
pt_adapter listFun [NB_FUN] = {adapter_exit, adapter_cd, adapter_pwd, adapter_mkdir, adapter_ls, adapter_rmdir};
int getFuncitonIndex(char *name){
    for(int i=0; i<NB_FUN; i++){
        if(strcmp(name, listCommande[i])==0)return i;
    }
    return -1;
}
int execSimpleCommande(tsh_memory *memory){
    resetCommand();
    fillArgs(memory->comand);
    int fun_index = getFuncitonIndex(args[0]);
    //check if in our command list
    if(fun_index<0){
        int pid_fils = fork();
        if(pid_fils==0){
            char **args2 = argsPlusNULL();
            execvp(args2[0], args2);
        }else{
            int status;
            waitpid(pid_fils, &status, WUNTRACED);
        }
    }else {//all the command in our list
        returnval = (*(listFun[fun_index]))(memory);//invok the appropriate function
    }
    return returnval;
}