#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include "simpleCommande.h"
#include "tsh_memory.h"
int save_write_fd;
int pipe_tsh(tsh_memory *memory1, tsh_memory *memory2){
    save_write_fd = dup(1);
    int fd_pipe[2];
    if(pipe(fd_pipe)==-1){
        perror("pipe:");
        return -1;
    }
    if(fork()){//parent writer
        close(fd_pipe[0]);
        dup2(fd_pipe[1], 1);
        close(fd_pipe[1]);
        execute(memory1);
        dup2(save_write_fd, 1);
    }else{//child read
        close(fd_pipe[0]);
        dup2(fd_pipe[0], 0);
        close(fd_pipe[0]);
        execute(memory2);
    }
    return 0;
}