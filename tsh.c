#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include "string_traitement.h"
#include "tsh_memory.h"
#include "cd.h"
#include "pwd.h"
#include "mkdir.h"
char * PATH;
tsh_memory * memory;
int main(void){
    //we create a memory about the current state so all processu can relate on it
    if((memory = create_memory())==NULL)return -1;
    /*while(1){
        PATH = getPath(memory);
        write(1, PATH, strlen(PATH));
        read(0, memory->comand, MAX_COMMAND);//user write his command on the input
        memory->comand[strlen(memory->comand)-1] = '\0';
        if(memmem(memory->comand, strlen(memory->comand), "exit", 4))break;
    }*/
    char * teste = "dossier1/dossier2/teste.tar/TP";
    char result[512];
    getPreTar(teste, result);
    printf("result = %s\n", result);
    free_tsh_memory(memory);
    return 0;
}