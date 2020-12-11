
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "cd.h"
#include "string_traitement.h"
#include "tsh_memory.h"

tsh_memory old_memory; //will be use to save/restore a memory

int right_redirection(tsh_memory *memory,int fd_tar, char *cmd, char *arg,char *target, int output, int append){
    if(is_unix_directory(target) == 1 || target[strlen(target)-1] == '/') return -1; //target given is a directory

    copyMemory(memory,&old_memory); //saving current state of the tsh_memory
    
    char *filename = target;
    char location[512];
    getLocation(target,location);
    if(strlen(location) > 0){
        if(cd(location,memory) == -1)//path given doesn't exist
            return -1;
        filename = target + strlen(location);
    } 

    int old_stdout = dup(STDOUT_FILENO);

    if(in_a_tar(memory) == 0){ //user wants the output file outside a tar
        int fd_target = 0;

        if(append == 0) 
            if( (fd_target = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0644) == -1)) return -1;
        else if(append == 1)
            if( (fd_target = open(filename, O_CREAT | O_APPEND | O_RDWR, 0644) == -1)) return -1;

        dup2(fd_target,STDOUT_FILENO);
        //execution de ls 
        dup2(old_stdout,STDOUT_FILENO);

        close(fd_target);
    } else { //user wants the output file inside a tar
        int pipefd[2];
        pipe(pipefd);
        int r = fork();
        if(r == 1){

        } else {
                
        }
    }
    close(old_stdout);
}