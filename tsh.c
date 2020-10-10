#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include "tsh_memory.h"
#include "cd.h"

#define BUFSIZE 512
tsh_memory * memory;
char read_buff[BUFSIZE]; //buff for the read

/*char ** getCommand(){//so we can pass it to exec
    char * debut_comande = read_buff + strlen(PATH);
    //comand arg1 arg2 ......
    //apres on cree juste un processus fils puis executer le programe
}*/
/*void update_path(int path_descriptor){
    lseek(path_descriptor, 0, SEEK_SET);
    read(path_descriptor, PATH, BUFSIZE);
}*/
int main(int nb, char **args){
    if(nb<2){ //if missing the .tar
        //should check also if it's not a .tar
        printf("missing the .tar file to execute this programe\n");
        return -1;
    }
    //we create a memory about the current state so all processu can relate on it
    if((memory = instanciate_tsh_memory("PATH.txt", args[1]))==NULL)return -1;

    /*while(1){*/
        update_path(memory);
        write(1, memory->PATH, strlen(memory->PATH));
        read(0, read_buff, BUFSIZE);//user write his command on the input
        read_buff[strlen(read_buff)-1] = '\0';
        //if(memmem(read_buff, strlen(read_buff), "exit", 4))break;
        cd("TP/TP3", memory);
        update_path(memory);
        write(1, memory->PATH, strlen(memory->PATH));
        read(0, read_buff, BUFSIZE);
        /*
            if the command wasn't exit
        */
    /*}*/
    free_tsh_memory(memory);
    return 0;
}