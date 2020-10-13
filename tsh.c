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

#define MAX_COMMAND 512
tsh_memory * memory;
char read_buff[MAX_COMMAND]; //buff for the read
int main(int nb, char **args){
    if(nb<2){ //if missing the .tar
        //should check also if it's not a .tar
        printf("missing the .tar file to execute this programe\n");
        return -1;
    }
    //we create a memory about the current state so all processu can relate on it
    if((memory = instanciate_tsh_memory(args[1]))==NULL)return -1;
    
    /*while(1){*/
        write(1, memory->PATH, strlen(memory->PATH));
        read(0, read_buff, MAX_COMMAND);//user write his command on the input
        read_buff[strlen(read_buff)-1] = '\0';
        //if(memmem(read_buff, strlen(read_buff), "exit", 4))break;

        cd("System", memory->PATH,memory->tar_descriptor);
    /*}*/
    //free_tsh_memory(memory);
    return 0;
}