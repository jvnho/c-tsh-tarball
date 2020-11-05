#include <string.h>
#include <stdio.h>
#include "tsh_memory.h"
#include "cd.h"
#include "pwd.h"
#include "mkdir.h"
#include "ls.h"
char *listCommande[] = {"cd", "pwd", "mkdir", "ls"};
#define NB_FUN 4
int getFuncitonIndex(char *name){
    for(int i=0; i<NB_FUN; i++){
        if(strcmp(name, listCommande[i])==0)return i;
    }
    return -1;
}
int execSimpleCommande(char *commande){
    printf("indice = %d\n", getFuncitonIndex("ls"));
    return 0;
}