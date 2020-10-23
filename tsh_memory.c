#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <stdio.h>
#include <string.h>
#include "tsh_memory.h"
#include "string_traitement.h"
void instanciate_tsh_memory(char *tar_file_name, tsh_memory *result){
    //instanciate the name of tar 
    strcpy(result->tar_name, tar_file_name);
    int len = strlen(result->tar_name);
    result->tar_name [len]= '/';
    result->tar_name [len+1]= '\0';
    //open the .tar file
    result->tar_descriptor = int_to_string(open(tar_file_name, O_RDWR));
    if(errno == ENOENT){//no such file
        perror("");
    }if(errno == EACCES){//permision dinied
        perror("");
    }
}
tsh_memory * create_memory(){
    tsh_memory * result = malloc(sizeof(tsh_memory));
    (result->FAKE_PATH)[0] = '\0';//so it doesn't create a random characteres
    result->tar_name[0] = '\0';
    return result;
}
char * getPath(tsh_memory *state){
    //get the pwd in the real path buffer and add '/'
    getcwd(state->REAL_PATH, sizeof(state->REAL_PATH)); 
    int len = strlen(state->REAL_PATH);
    state->REAL_PATH[len] = '/';
    state->REAL_PATH[len+1] = '\0';//avoid the random characteres

    if(strlen(state->tar_name)!=0){
        //concat Real + tar_name + fake (before calling strcat make sure the first string has '\0')
        strcat(state->REAL_PATH, state->tar_name);//concat with the tar directory
        state->REAL_PATH[strlen(state->REAL_PATH)] = '\0';
        strcat(state->REAL_PATH, state->FAKE_PATH);
        state->REAL_PATH[strlen(state->REAL_PATH)] = '\0';
    }
    //add a $ at the end
    int length = strlen(state->REAL_PATH);
    state->REAL_PATH[length] = '$';
    state->REAL_PATH[length+1] = ' ';
    state->REAL_PATH[length+2] = '\0';
    return state->REAL_PATH;
}
void free_tsh_memory(tsh_memory *state){//at the end
    //close(string_to_int(state->tar_descriptor)); 
    free(state);
}
int in_a_tar(tsh_memory *state){
    if(strlen(state->FAKE_PATH))return 1;
    return 0;
}
