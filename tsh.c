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
    if((memory = instanciate_tsh_memory("PATH.txt", args[1]))==NULL)return -1;
    //cd sdfqjmf ->cd fdsfqsd
    /*while(1){*/
        update_path(memory);
        write(1, memory->PATH, strlen(memory->PATH));
        read(0, read_buff, BUFSIZE);//user write his command on the input
        read_buff[strlen(read_buff)-1] = '\0';
        //if(memmem(read_buff, strlen(read_buff), "exit", 4))break;
        getCommand(to_array_of_string(&memory->tail_comand, read_buff, int_to_string(memory->path_descriptor), int_to_string(memory->tar_descriptor)), memory);
        int pid_fils = fork();
        int info;
        /*if(pid_fils == 0){
            printf("rentre dans fils\n");
            //cd(memory->comand[1], memory->comand[2], memory->comand[3]);
            if(execvp(memory->comand[0], memory->comand)==-1){
                perror("erreur fils :");
            }
        }else{
            printf("rentre dans pere");
            waitpid(pid_fils, &info, WUNTRACED);
        }*/
        
    /*}*/
    //free_tsh_memory(memory);
    return 0;
}