#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"

#include "cd.h"


tsh_memory old_memory;


int stopCat = 0; // stop the cat when giver without argument

void cat_in_tar(int fd, char* PATH){ //test tar
    lseek(fd,0,SEEK_SET);
    struct posix_header hd;
    memset(&hd,0,512);

    while(read(fd,&hd,BLOCKSIZE) > 0){
        int file_s = 0;
        sscanf(hd.size,"%o", &file_s);
        int nb_bloc_fichier = (file_s + 512 -1) / 512;
        if(strcmp(hd.name,PATH)==0){
            char buffer[nb_bloc_fichier*512];
            read(fd, buffer, BLOCKSIZE);
            write(1,buffer,strlen(buffer));
        }

        //next header block
        lseek(fd,nb_bloc_fichier*512,SEEK_SET);

    }
    
    
}

//case of the cat where the user gives no arguement
void exitFromCat(int signal){
    int pipe_fd[2];
    pipe(pipe_fd);
    int pid = fork();
    char buffer[1024];
    memset(buffer,0,1024);
    if(pid == 0){
        close(pipe_fd[0]);
        while(read(0,buffer,1024) > && stopCat == 0){
            write(pipe_fd[1], buffer, strlen(buffer));
            memset(buffer,0,1024);
        }
        close(pipe_fd[1]);
        exit(0);
        
    }
    else{
        close(pipe_fd[1]);
        while(read(pipe_fd[0],buffer,1024) > 0 && stopCat == 0){
            write(1,buffer,1024);
            memset(buffer,0,1024);

        }
        close(pipe_fd[0])

        //redefining SIGINT singal behaviour to default
        sigaction(SIGINT,&old_cat,NULL);
        return 1;
    }
}
