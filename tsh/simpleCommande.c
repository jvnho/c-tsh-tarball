#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "tsh_memory.h"
#include "simpleCommande.h"
#include "cd.h"
#include "pwd.h"
#include "mkdir.h"
#include "ls.h"
#include "rmdir.h"
#include "string_traitement.h"
#include "rm.h"
#include "redirection.h"
#include "cp.h"
#include "mv.h"
#include "pipe.h"
#include "cat.h"

char *listCommande[] = {"exit", "cd", "pwd", "mkdir", "ls", "rmdir", "rm", "cp", "mv", "cat"};
#define NB_FUN 10
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
void freeArgPlusNull(char **result){
    int len = i_option + i_args + 2;
    for(int i = 0; i< len; i++){
        free(result[i]);
    }
    free(result);
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
    if(i_option)return mkdir_tar(option ,args, i_option, i_args, memory);
    return mkdir_tar(NULL ,args, i_option, i_args, memory);
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
int adapter_cp(tsh_memory *memory){
    if(i_option)return copy_tar(option, args, i_option, i_args, memory);
    return copy_tar(NULL, args, i_option, i_args, memory);
}
int adapter_mv(tsh_memory *memory){
    return mv(memory, args, i_args, option, i_option);
}
int adapter_cat(tsh_memory *memory){
    return cat(memory,args,i_args,option,i_option);
}

typedef int (*pt_adapter) (tsh_memory *memory);//declaration pointer of function
pt_adapter listFun [NB_FUN] = {adapter_exit, adapter_cd, adapter_pwd, adapter_mkdir, adapter_ls, adapter_rmdir, adapter_rm, adapter_cp, adapter_mv, adapter_cat};

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
        char **args2 = argsPlusNULL();
        int pid_fils = fork();
        if(pid_fils==0){
            if(strlen(args2[0])==0)exit(130);//no command passed
            if(execvp(args2[0], args2) == -1)exit(127);//command not found
        }else{
            freeArgPlusNull(args2);
            int status;
            waitpid(pid_fils, &status, WUNTRACED);
            if(WIFEXITED(status)){//if the child used exit
                return WEXITSTATUS(status);//0 ok, 1 error
            }//how about a signal?
        }
    }else {//all the command in our list
        return (*(listFun[fun_index]))(memory);//invok the appropriate function
    }
    return 0;
}
/*
void printError(tsh_memory *memory, int error){
    char message[538];
    memset(message, 0, 538);
    sprintf(message, "tsh : command not found: %s\n", memory->comand);
    if(error == 127)write(2, message, strlen(message));
}*/

int execute(tsh_memory *memory){
    if(strstr(memory->comand, "|") != NULL) //Pipe found in command line
    {
        tsh_memory mem1;
        tsh_memory mem2;
        if(spilitPipe(memory, &mem1, &mem2) == -1)
        {
            write(2, "parse error near `|'\n", strlen("parse error near `|'\n"));
            return 1;
        }
        return pipe_tsh(&mem1, &mem2);
    } else if(strstr(memory->comand,"<") != NULL || strstr(memory->comand,">") != NULL)
    {
        return redirection(memory);
    }
    return execSimpleCommande(memory);
}