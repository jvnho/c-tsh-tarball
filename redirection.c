#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
//#include <cp.h>
#include <wait.h> //temporaire

#include "simpleCommande.h"
#include "tar.h"
#include "redirection.h"
#include "cd.h"
#include "string_traitement.h"
#include "tsh_memory.h"
#include "rm.h"
#include "exec_funcs.h"

tsh_memory old_memory; //will be use to save/restore a memory

void convert_to_simple_cmd(tsh_memory *memory){ //removes redirection symbol from the tsh_memory "comand"
    char *cmd = strdup(memory->comand); //command line entered by user
    char new_cmd[strlen(cmd)];
    new_cmd[0] = '\0';

    char *tok = strtok(cmd, " ");
    while(tok != NULL){
        if((strstr(tok,">") != NULL) || (strstr(tok, "2>") != NULL)) break;
        if(strstr(tok,"<") == NULL){
            strcat(new_cmd, tok);
            strcat(new_cmd, " ");   
        }
        tok = strtok(NULL, " "); 
    }
    new_cmd[strlen(new_cmd)] = '\0';
    strcpy(memory->comand, new_cmd);
}

int fill_redir_array(tsh_memory *memory, redirection_array *data, char *str, int output, int append){
    if(is_unix_directory(str) == 1 || str[strlen(str)-1] == '/') return -1; //redirection given is a directory so we abort

    char *redir_name = str;
    char location[512];
    memset(location,0, 512);
    getLocation(redir_name, location);
    copyMemory(memory, &old_memory);//Keeps the curent state of the programm
    if(strlen(location) > 0){
        if(cd(location, memory) == -1) return -1; //if one of the redirection is incorrect we abort
        redir_name += strlen(location);
    } else if(strlen(location) == 0 && strstr(redir_name, ".tar")){
        if(cd(str, memory) == -1) return -1; //same
    }
    //starting to fill
    int index = data->NUMBER;
    if(in_a_tar(memory) == 1){
        data->IN_A_TAR[index] = 1;
        //NAME
        if(strlen(location) > 0) strcpy(data->NAME[index], redir_name);
        else strcpy(data->NAME[index], str);
        //REDIR_PATH
        char *cur_full_path = getPath(memory);
        cur_full_path[strlen(cur_full_path)-2] = '\0'; //removing'$'
        char tar_path[512]; //will be the header path of the tar file
        get_tar_path(memory,tar_path, cur_full_path); //check @string_traitement.c
        strcpy(data->REDIR_PATH[index],tar_path);
    } else {
        data->IN_A_TAR[index] = 0;
        strcpy(data->REDIR_PATH[index], str);
    }
    //MISCELLANEOUS
    data->STD[index] = output;
    data->APPEND[index] = append;
    data->NUMBER++;
    restoreLastState(old_memory, memory);
    return 0;
}

struct redirection_array* associate_redirection(tsh_memory *memory, char *cmd){ //associating every redirection to the standard output/error interested by user
    struct redirection_array *data = malloc(sizeof(redirection_array)); // check @redirection.h
    char *tok = strtok(cmd," ");
    while(tok != NULL){
        if(strstr(tok, "2>") != NULL){
            if(strcmp(tok, "2>&1") == 0){
                tok = strtok(NULL, " "); //next token
                if(fill_redir_array(memory, data, tok, 3, 1) ==-1) return NULL; //one of the redirection path given doesn't exist
                return data; //we consider user cant use 2>&1 with other redirection (no sense)
            } else {
                int append;
                if((tok+2)[0] == '>') append = 1; //2>>
                else append = 0;
                tok = strtok(NULL, " "); //next token
                if(fill_redir_array(memory, data, tok, 2,append) == -1) return NULL;
            }
        } else if(strstr(tok, ">") != NULL){
            int append;
            if( (tok+1)[0] == '>') append = 1; //>>
            else append = 0;
            tok = strtok(NULL, " "); //next token
            if(fill_redir_array(memory, data, tok, 1,append) ==-1) return NULL;
        } 
        else tok = strtok(NULL," ");
    }
    return data;
}

void delete_file(char *target){ //temporary
    int pid = fork();
    if(pid == 0){ //child
        execlp("rm", "rm", target, NULL);
        exit(0);
    } else { //parent   
        waitpid(pid, NULL, 0);
    }
}

int redirection(tsh_memory *memory){
    struct redirection_array *data = associate_redirection(memory, strdup(memory->comand)); 
    if(data == NULL) return -1;
    if(data->NUMBER > 2) return -1;
    convert_to_simple_cmd(memory); //Converting command line entered by user to make it readable by the programm

    int fd_fic_out = 0, fd_fic_err = 0, fd_fic_mix = 0;//Respectively file descriptor than will receive redirection from >, 2>, 2>&1
    int old_stdout = dup(STDOUT_FILENO), old_stderr = dup(STDERR_FILENO); 

    for(int i = 0; i < data->NUMBER; i++){ //Going through this loop at most 2 times 
        char *file_path;
        if(data->IN_A_TAR[i] == 0) file_path = data->REDIR_PATH[i];
        else file_path = data->NAME[i]; //creates a file just outside of the tar (it will be relocated)
        int append = data->APPEND[i];
        if(data->STD[i] == 3){ //User wants a "2>&1" redirection 
            fd_fic_mix = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            assert(fd_fic_mix != -1);
        } 
        else if(data->STD[i] == 2){ //User wants a "2>" redirection (maybe 2>> if append == 1)
            if(append == 1) fd_fic_err = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_err = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_err != -1);
        } 
        else { //User wants a ">" redirection (maybe 2>> if append == 1)
            if(append == 1) fd_fic_out = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_out = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_out != -1);
        }
        if(fd_fic_mix != 0){ //If we opened a file to receive 2>&1
            dup2(fd_fic_mix, STDOUT_FILENO);
            dup2(fd_fic_mix, STDERR_FILENO);
        } else {
            if(fd_fic_out != 0){ //If we opened a file to receive >
                dup2(fd_fic_out, STDOUT_FILENO);
            }
            if(fd_fic_err != 0){ //If we opened a file to receive 2>
                dup2(fd_fic_err, STDERR_FILENO);
            }
        }
    }
    execSimpleCommande(memory); //output and/or errors will be written in open file
    //close eventual open files
    if(fd_fic_out != 0) close(fd_fic_out);
    if(fd_fic_err != 0) close(fd_fic_err);
    if(fd_fic_mix != 0) close(fd_fic_mix);

    //restoring stdout and stderr
    dup2(old_stdout, STDOUT_FILENO);
    dup2(old_stderr, STDERR_FILENO);
    for(int i = 0; i < data->NUMBER; i++){ //Now we move file to the tar if wanted by user
        if(data->IN_A_TAR[i] == 1){ //redirection file was supposed to be in a tar so we move it
            //cp_file_tar(data->NAME[i], data->REDIR_PATH[i], atoi(memory->tar_descriptor));
            delete_file(data->NAME[i]);
        } else continue;
    }
    return 1;
}