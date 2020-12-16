#include <stdio.h>
#include <unistd.h>
#include "tsh_memory.h"
#include "simpleCommande.h"
int save_read_fd;
int pipe_tsh(tsh_memory *memory1, tsh_memory *memory2){
    
    save_read_fd = dup(0);
    int fd_pipe[2];
    if(pipe(fd_pipe)==-1){
        perror("pipe:");
        return -1;
    }
    int pid_fils = fork();
    if(pid_fils){//Parent (read)
        close(fd_pipe[1]);
        dup2(fd_pipe[0], 0);
        close(fd_pipe[0]);
        int status;
        waitpid(pid_fils, &status, 0);
        if(WIFEXITED(status)){//if the child used exit()
            if(WEXITSTATUS(status)){//if the number passed by exit is not 0, then the child finished with error
                dup2(save_read_fd, 0);
                printf("here = %d\n", WEXITSTATUS(status));
                return WEXITSTATUS(status);//report the error of my child to my parent
            }
        }
        int result = execute(memory2);
        dup2(save_read_fd, 0);
        return result;
    }else{//child (write)
        close(fd_pipe[0]);
        dup2(fd_pipe[1], 1);
        close(fd_pipe[1]);
        exit(execute(memory1));//report the result to parent who is waiting
    }
    return 0;
}