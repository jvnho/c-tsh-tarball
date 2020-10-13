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

tsh_memory * memory;
int main(void){
    //we create a memory about the current state so all processu can relate on it
    if((memory = create_memory())==NULL)return -1;
    while(1){
        write(1, memory->FAKE_PATH, strlen(memory->FAKE_PATH));
        read(0, memory->comand, MAX_COMMAND);//user write his command on the input
        memory->comand[strlen(memory->comand)-1] = '\0';
        if(memmem(memory->comand, strlen(memory->comand), "exit", 4))break;
    }
    free_tsh_memory(memory);
    return 0;
}