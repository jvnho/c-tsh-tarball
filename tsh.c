#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#define BUFSIZE 512
char read_buff[BUFSIZE]; //buff for the read

int main(int nb, char **args){
    if(nb<2){ //if missing the .tar
        printf("missing the .tar file to execute this programe\n");
        return -1;
    }
    while(1){
        read(0, read_buff, BUFSIZE);//user write his command on the input
        read_buff[strlen(read_buff)-1] = '\0';
        if(strcmp("exit", read_buff)==0)break;
        /*
            if the command wasn't exit
        */
    }
    return 0;
}