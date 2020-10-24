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
    PATH = getPath(memory);
    write(1, PATH, strlen(PATH));
    /*
        change le chemin sur ton truc a toi 
        essaye
        cd cheminNormal
        cd cheminNormal/dossier.tar
        cd dossier.tar
        cd dossier.tar/trucDansTar
        cd cheminNormal/dossier.tar/trucDans tar
    */
    printf("result cd = %d\n", cd("test2/test3.tar/doc/", memory)); 
    PATH = getPath(memory);//affiche le path après le cd
    write(1, PATH, strlen(PATH));
    mkdir("dossMkdir", memory);
    free_tsh_memory(memory);
    return 0;
}