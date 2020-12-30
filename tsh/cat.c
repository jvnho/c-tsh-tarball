#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <wait.h>
#include <fcntl.h> 
#include <signal.h>

#include "exec_funcs.h"
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"
#include "cd.h"

tsh_memory old_memory; //will be use to save/restore a memory
char **array_execvp;


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
            read(fd, buffer, BLOCKSIZE*nb_bloc_fichier);
            write(1,buffer,strlen(buffer));
        }

        //next header block
        lseek(fd,nb_bloc_fichier*512,SEEK_CUR);

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
    int pid = fork();
    if(pid == 0){
        execlp("cat", "cat", NULL);
        exit(0);
    }
    else{
        waitpid(pid, NULL, 0); 
        write(1, "\n", 1);
    }
    //redefining SIGINT singal behaviour to default
    sigaction(SIGINT,&old_act,NULL);
    return 1;
}


int cat(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50], int nb_option){
    if(nb_arg == 0){
        //redefining SIGINT signal behaviour
        struct sigaction action, old_act;
        memset(&action, 0,sizeof(action));
        memset(&old_act, 0, sizeof(old_act));
        action.sa_handler = exitFromCat;
        sigaction(SIGINT,&action, &old_act);
        return exec_cat(old_act);
    }
    tsh_memory old_memory;
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
              cat_in_tar(atoi(memory->tar_descriptor), path_to_target);
              free(path_to_target);
          }
          else{
              array_execvp = execvp_array("cat", fileToCat, option, nb_option);
              exec_cmd("cat",array_execvp);

          }
          restoreLastState(old_memory,memory);

    }
    return 1;


}


   

       
