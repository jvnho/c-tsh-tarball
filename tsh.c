#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>

#define BUFSIZE 512
char read_buff[BUFSIZE]; //buff for the read
char PATH[1024];//representing the relative path from the tar

void startPath(char *arg){//instanciate the relative path
    int arg_size = strlen(arg);
    strcpy(PATH, arg);
    PATH[arg_size] = '/';
    PATH[arg_size+1] = '\0';
}

int main(int nb, char **args){
    if(nb<2){ //if missing the .tar
        //should check also if it's not a .tar
        printf("missing the .tar file to execute this programe\n");
        return -1;
    }
    //we open our .tar in order to execute command on it
    int fd_tar = open(args[1], O_RDWR);
    //start the path as .tar directory 
    if(errno == ENOENT){//no such file
        perror("");
        return -1;
    }if(errno == EACCES){//permision dinied
        perror("");
        return -1;
    }
    startPath(args[1]);
    while(1){
        write(1, PATH, strlen(PATH));
        read(0, read_buff, BUFSIZE);//user write his command on the input
        read_buff[strlen(read_buff)-1] = '\0';
        if(memmem(read_buff, strlen(read_buff), "exit", 4))break;
        /*
            if the command wasn't exit
        */
    }
    close(fd_tar);
    return 0;
}