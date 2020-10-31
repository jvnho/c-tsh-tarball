#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <time.h>
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"


int mkdir_in_tar(char *dir_name, int tar_descriptor){
    struct posix_header *new_head = create_header(dir_name);
    put_at_the_first_null(tar_descriptor);
    if(write((tar_descriptor), new_head, 512)==-1){//write on the first ending bloc
        perror("");
        return -1;
    }
    //add the last bloc of zero at the end
    lseek(tar_descriptor, 0, SEEK_END);
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    if(write(tar_descriptor, end_bloc, 512)==-1){
        perror("");
        return -1;
    }
    return 0;
}
//to do with more than one argument
int mkdir(char *dir_name, tsh_memory *memory){
    if(in_a_tar(memory)){//in tar -> so use our implementation of mkdir
        return mkdir_in_tar(concatString(memory->FAKE_PATH, dir_name), string_to_int(memory->tar_descriptor));
    }else{//normal circonstances so we exec the normal mkdir
        int pid = fork();
        if(pid==0){//child
            execlp("mkdir", "mkdir", dir_name, NULL);
        }else{//parent
            int status;
            waitpid(pid, &status, WUNTRACED);
            if(WEXITSTATUS(status)==-1)return -1;
        }
    }
    return 0;
}
