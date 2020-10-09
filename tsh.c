#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include "cd.h"

#define BUFSIZE 512
char read_buff[BUFSIZE]; //buff for the read
char PATH[1024];//representing the relative path from the tar
char *getCommand(){
    return read_buff + strlen(PATH);
}
void update_path(int path_descriptor){
    lseek(path_descriptor, 0, SEEK_SET);
    read(path_descriptor, PATH, BUFSIZE);
}
int main(int nb, char **args){
    if(nb<2){ //if missing the .tar
        //should check also if it's not a .tar
        printf("missing the .tar file to execute this programe\n");
        return -1;
    }
    //we open our .tar in order to execute command on it
    int fd_tar = open(args[1], O_RDWR);
    int fd_path = open("PATH.txt", O_RDWR);
    if(errno == ENOENT){//no such file
        perror("");
        return -1;
    }if(errno == EACCES){//permision dinied
        perror("");
        return -1;
    }

    while(1){
        update_path(fd_path);
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