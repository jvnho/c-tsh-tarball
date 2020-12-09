#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tsh_memory.h"
#include "cd.h"
#include "pwd.h"
#include "mkdir.h"
#include "ls.h"
#include "rmdir.h"
#include "string_traitement.h"
#include "rm.h"
#include "pipe.h"



char *listCommande[] = {"exit", "cd", "pwd", "mkdir", "ls", "rmdir", "rm"};
#define NB_FUN 7
char co[50];
char option[50][50];
int i_option = 0;
char args[50][50];
int i_args = 0;
const char space[2] = " ";
int returnval;
int pipe_tsh(tsh_memory *m, tsh_memory *a);

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
void fillOptions(char *commande){
    if(strstr(commande, "-")==NULL){
        fillArgs(commande);
    }else{

        char com[512];
        strcpy(com, commande);
        com[strlen(commande)] = '\0';
        char *read;
        read = strtok(com, space);
        int size_options = 0;
        while(read != NULL){

            if(read[0] == '-'){//an option
                strcpy(option[i_option], read);
                option[i_option][strlen(read)] = '\0';
                i_option++;
                size_options = size_options + strlen(read) + 1;
                if(read[1]=='m'){//mode
                    read = strtok(NULL, space);
                    strcpy(option[i_option], read);
                    option[i_option][strlen(read)] = '\0';
                    i_option++;
                    size_options = size_options + strlen(read) + 1;
                }
            }else{
                fillArgs(commande + size_options);
            }
            read = strtok(NULL, space);

        }
    }

}
void fillCo(char *commande){
    char com[512];
    strcpy(com, commande);
    com[strlen(commande)] = '\0';
    char *read;
    read = strtok(com, space);
    if(read!=NULL){
        strcpy(co, read);
        co[strlen(read)] = '\0';
    }
    fillOptions(commande + (strlen(co) + 1));
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
    //command
    assert((result = malloc((i_option + i_args + 2) * sizeof(char *))) != NULL);
    assert((result[0] = malloc(strlen(co)*sizeof(char))) != NULL);
    strcpy(result[0], co);
    int index_result = 1;
    for(int i = 0; i< i_option; i++){
        assert((result[index_result] = malloc(strlen(option[i])*sizeof(char))) != NULL);
        strcpy(result[index_result], option[i]);
        index_result++;
    }
    for(int i = 0; i<i_args; i++){
        assert((result[index_result] = malloc(strlen(args[i])*sizeof(char))) != NULL);
        strcpy(result[index_result], args[i]);
        index_result++;
    }
    result[index_result] = NULL;
    return result;
}
//Adapter Pattern
int adapter_exit(tsh_memory *memory){
    return exit2(memory);
}
int adapter_cd(tsh_memory *memory){
    return cd(args[0], memory);
}
int adapter_pwd(tsh_memory *memory){
    return pwd(memory);
}
int adapter_mkdir(tsh_memory *memory){
    if(i_option)return mkdir(option ,args, i_option, i_args, memory);
    return mkdir(NULL ,args, i_option, i_args, memory);
}
int adapter_ls(tsh_memory *memory){
    return ls(memory,args,i_args,option,i_option);
}
int adapter_rmdir(tsh_memory *memory){
    return rmdir_func(memory, args, i_args, option, i_option);
}
int adapter_rm(tsh_memory *memory){
    return rm(memory, args, i_args, option, i_option);
}

typedef int (*pt_adapter) (tsh_memory *memory);//declaration pointer of function
pt_adapter listFun [NB_FUN] = {adapter_exit, adapter_cd, adapter_pwd, adapter_mkdir, adapter_ls, adapter_rmdir, adapter_rm};

int getFuncitonIndex(char *name){
    for(int i=0; i<NB_FUN; i++){
        if(strcmp(name, listCommande[i])==0)return i;
    }
    return -1;
}

int execSimpleCommande(tsh_memory *memory){
    resetCommand();
    fillCo(memory->comand);

    int fun_index = getFuncitonIndex(co);

    if(fun_index<0){
        int pid_fils = fork();
        if(pid_fils==0){
            char **args2 = argsPlusNULL();//ou mettre free?
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
int execute(tsh_memory *memory){
    if(strstr(memory->comand, "|")==NULL){//Pas de pipe
        tsh_memory mem1;
        tsh_memory mem2;
        copyMemory(memory, &mem1);
        copyMemory(memory, &mem2);
        execSimpleCommande(memory);
    }else{
        write(1, "avec pipe\n", strlen("avec pipe\n"));
    }
    //pipe(memory, memory);
    return 0;
}