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
#include "ls.h"
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
        change le chemin du cd sur ton truc a toi
        essaye
            cd cheminNormal
            cd cheminNormal/dossier.tar
            cd dossier.tar
            cd dossier.tar/trucDansTar
            cd cheminNormal/dossier.tar/trucDans tar
        Comme il y la ligne mkdir après le cd normalement a chaque fois tu va avoir un nouveau dossier dossMkdir
    */
    printf("resultat du cd = %d\n", cd("vide/", memory)); //*** cd ***/
    PATH = getPath(memory);//affiche le path après le cd
    write(1, PATH, strlen(PATH));
    mkdir("newdoss", memory);//cree un dossMKdir dans l'emplacement ou on a fait le cd
    ls(memory);
    free_tsh_memory(memory);
    return 0;
}
