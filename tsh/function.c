#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "tsh_memory.h"
#include "cd.h"

void restoreLastState(tsh_memory old_memory, tsh_memory *memory){
    copyMemory(&old_memory, memory); //restoring the last state of the memory
    char *destination = malloc(strlen(memory->REAL_PATH));
    strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
    cd(destination,memory); //cd-ing back to where we were
    free(destination);
}

//making an array for execvp
char** execvp_array(char *cmd, char *dir, char option[50][50], int nb_option){
    char **ret;
    assert((ret = malloc(sizeof(char*) * (nb_option+3))) != NULL);

    assert((ret[0] = malloc(sizeof(char) * strlen(cmd))) != NULL);
    strcpy(ret[0], cmd);
    
    int index = 1;
    for(int i = 0; i < nb_option; i++){
        assert((ret[index] = malloc(sizeof(char)*strlen(option[i]))) != NULL);
        strcpy(ret[index], option[i]);
        index++;
    }
    if(dir != NULL){
        assert((ret[index] = malloc(sizeof(char)*strlen(dir))) != NULL);
        strcpy(ret[index], dir);
        index++;
    }
    ret[index] = NULL;
    return ret;
}

void exec_cmd(char *cmd, char **args){
    int r = fork();
    if(r == 0) execvp(cmd, args);
    else wait(NULL);
}

//check if a certain "opt_name" is present in "option" array
int option_present(char *opt_name, char option[50][50], int nb_option){
    for(int i = 0; i < nb_option; i++)
        if(strcmp(option[i], opt_name) == 0)
            return 1;
    return 0;
}