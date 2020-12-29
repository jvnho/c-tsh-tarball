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
char **array_execvp;


//int stopCat = 0; // stop the cat when giver without argument

int stopCat = 0;

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


void exitFromCat(int signal){
    if(signal == SIGINT){
         stopCat = 1;
    }
    else {
        exit(-1);
    }
    
}


//case of the cat where the user gives no arguement
int exec_cat(struct sigaction old_act){
    int pipe_fd[2];
    pipe(pipe_fd);
    int pid = fork();
    char buffer[1024];
    memset(buffer,0,1024);
    if(pid == 0){
        close(pipe_fd[0]);
        while(read(0,buffer,1024) > && stopCat == 0){ // don't work
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
        close(pipe_fd[0]);

        //redefining SIGINT singal behaviour to default
        
        //sigaction(SIGINT,&old_cat,NULL);
        return 1;
    }
}


int cat(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50], int nb_option){
    if(nb_arg == 0){
        //redefining SIGINT signal behaviour
        struct sigaction action, old_act;
        memset(&action, 0,sizeof(action));
        memset(&old_act, 0, sizeof(old_act));
        action.sa_handler = exitFromCat;
        signaction(SIGINT,&action, &old_act);
        return exec_cat(old_act);
    }
    
    for(int i = 0; i < nb_arg; i++){
        //start
        char *fileToCat = args[i];
        if(fileToCat[strlen(fileToCat -1)] == '/' || is_unix_directory(fileToCat)== 1) continue; //lose directory
          copyMemory(memory,&old_memory); // saving current state of tsh memory

          char location[512];
          memset(location, 0,512);
          getLocation(fileToCat, location); //

          if(strlen(location)> 0){
              if(cd(location,memory)== -1) continue; // when user give incorrect path (problem with cd function)
              fileToCat += strlen(location);
          }
          if(in_a_tar(memory)==1){
              char *path_to_target = concate_string(memory->FAKE_PATH, fileToCat);
              cat_in_tar(ato1(memory->tar_descriptor), path_to_target);
              free(path_to_target);
          }
          else{
              array_execvp = execvp_array("cat", fileToCat, option, nb_option);
              exec_cmd("cat",array_execvp);

          }
          restoredLastState(old_memory,memory);

    }
    return 1;


}
