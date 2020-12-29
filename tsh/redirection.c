#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>

#include "redirection.h"
#include "simpleCommande.h"
#include "cd.h"
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"
#include "exec_funcs.h"
#include "mv.h"

//check if redirection entered by user make sense ex: "ls > fic >> fic2 2&>1" doesn't make sense
//analysing the "redirection_array" struct
int is_redirection_valid(redirection_array *data){
    if(data->NUMBER > 2) return -1;
    int nb_stdout = 0, nb_stderr = 0, nb_both = 0; //number of redirection of each (>/>>; 2>/2>>; 2>&1)
    for(int i = 0; i < data->NUMBER; i++)
    {
        if(data->STD[i] == 1) nb_stdout++;
        else if(data->STD[i] == 2) nb_stderr++;
        else if(data->STD[i] == 3) nb_both++;
    }
    if(nb_stdout < 2 && nb_stderr < 2 && nb_both < 2) return 1;
    else return -1;
}

//removes redirection symbol from the tsh_memory "comand"
//ex: "ls > fic 2> fic2" --> "ls"
void convert_to_simple_cmd(tsh_memory *memory){ 
    char *cmd = strdup(memory->comand); //command line entered by user
    char new_cmd[strlen(cmd)];
    new_cmd[0] = '\0';

    char *tok = strtok(cmd, " ");
    while(tok != NULL)
    {
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

int fill_redir_array(tsh_memory *memory, redirection_array *data, char *target, int output, int append){
    if(is_unix_directory(target) == 1 || target[strlen(target)-1] == '/') return -1; //redirection given is a directory so we abort

    tsh_memory old_memory;
    char *redir_name = target;
    char location[512];
    getLocation(redir_name, location);
    int lenLocation = strlen(location);
    copyMemory(memory, &old_memory);//Keeps the curent state of the programm
    if(lenLocation > 0)
    {
        if(cd(location, memory) == -1) return -1; //if one of the redirection is incorrect we abort
        redir_name += strlen(location);
    }
    else if(lenLocation == 0 && strstr(redir_name, ".tar"))
    {
        if(cd(target, memory) == -1) return -1; //same
    }
    //starting to fill
    int index = data->NUMBER;
    if(in_a_tar(memory) == 1)
    {
        data->IN_A_TAR[index] = 1;
        if(lenLocation > 0) 
            strcpy(data->NAME[index], redir_name);
        else 
            strcpy(data->NAME[index], target);
        char *cur_full_path = getPath(memory);
        cur_full_path[strlen(cur_full_path)-2] = '\0'; //removing'$'
        char tar_path[512]; //will be the header path of the tar file
        concatenationPath(memory->tar_name, "/", tar_path);
        get_tar_path(memory, cur_full_path, tar_path); //check @string_traitement.c
        strcpy(data->REDIR_PATH[index],tar_path);
    } else {
        data->IN_A_TAR[index] = 0;
        strcpy(data->REDIR_PATH[index], target);
    }
    data->STD[index] = output;
    data->APPEND[index] = append;
    data->NUMBER++;
    restoreLastState(old_memory, memory);
    return 0;
}

//associating every redirection to the standard output/error interested by user
struct redirection_array* associate_redirection(tsh_memory *memory, char *cmd){ 
    struct redirection_array *data; //Check @redirection.h
    assert(( data = malloc(sizeof(redirection_array))) != NULL);
    memset(data,0,sizeof(redirection_array)); 
    char *tok = strtok(cmd," ");
    while(tok != NULL)
    {
        if(strstr(tok, "2>") != NULL)
        {
            if(strcmp(tok, "2>&1") == 0){
                tok = strtok(NULL, " "); //next token
                if(fill_redir_array(memory, data, tok, 3, 1) ==-1) return NULL; //one of the redirection path given doesn't exist
                return data; //we consider user cant use 2>&1 with other redirection (no sense)
            } else {
                int append = 0;
                if((tok+2)[0] == '>') append = 1; //2>>
                tok = strtok(NULL, " "); //next token
                if(fill_redir_array(memory, data, tok, 2,append) == -1) return NULL;
            }
        } 
        else if(strstr(tok, ">") != NULL)
        {
            int append = 0;
            if( (tok+1)[0] == '>') append = 1; //>>
            tok = strtok(NULL, " "); //next token
            if(fill_redir_array(memory, data, tok, 1,append) ==-1) return NULL;
        } 
        else tok = strtok(NULL," ");
    }
    return data;
}

void print_date(redirection_array *data){
    for(int i = 0 ; i < data->NUMBER; i++){
        printf("NAME: %s\n", data->NAME[i]);
        printf("STD %d \n", data->STD[i]);
        printf("APPEND %d \n", data->APPEND[i]);
        printf("IN_A_TAR %d\n", data->IN_A_TAR[i]);
        printf("REDIR_PATH %s \n", data->REDIR_PATH[i]);
        printf("NUMBER %d \n", data->NUMBER);
    }
}

int redirection(tsh_memory *memory){
    struct redirection_array *data = associate_redirection(memory, strdup(memory->comand)); 
    if(data == NULL || is_redirection_valid(data) == -1)
    {
        return -1; //Redirection given by user doesn't make sense so we abort.
    }
    convert_to_simple_cmd(memory); //Converting command line entered by user to make it readable by the programm

    int fd_fic_out = 0, fd_fic_err = 0, fd_fic_mix = 0;//Respectively file descriptor than will receive redirection from >, 2>, 2>&1
    int old_stdout = dup(STDOUT_FILENO), old_stderr = dup(STDERR_FILENO); 

    for(int i = 0; i < data->NUMBER; i++) //Going through this loop at most 2 times 
    { 
        char *file_path;
        if(data->IN_A_TAR[i] == 0){
            file_path = data->REDIR_PATH[i];
        } else {
            file_path = data->NAME[i]; //creates a file just outside of the tar (it will be relocated)
        }
        int append = data->APPEND[i];
        if(data->STD[i] == 3) //User wants a "2>&1" redirection 
        { 
            fd_fic_mix = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            assert(fd_fic_mix != -1);
        } 
        else if(data->STD[i] == 2) //User wants a "2>" redirection (maybe 2>> if append == 1)
        { 
            if(append == 1) fd_fic_err = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_err = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_err != -1);
        } 
        else //User wants a ">" redirection (maybe 2>> if append == 1)
        { 
            if(append == 1) fd_fic_out = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_out = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_out != -1);
        }

        if(fd_fic_mix != 0){ //If we opened a file to receive 2>&1
            dup2(fd_fic_mix, STDOUT_FILENO);
            dup2(fd_fic_mix, STDERR_FILENO);
        } 
        else 
        {
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

    //Procedure to move the file into the tar in needed
    for(int i = 0; i < data->NUMBER; i++) 
    { 
        if(data->IN_A_TAR[i] == 1){ //redirection file was supposed to be in a tar so we move it
            char path_to_src[512]; //src file is located in the working directory
            getcwd(path_to_src, 512);
            strcat(path_to_src, "/");
            strcat(path_to_src, data->NAME[i]);
            printf("src %s\n", path_to_src);
            printf("target %s\n", data->REDIR_PATH[i]);
            do_mv(memory, path_to_src,data->REDIR_PATH[i], NULL, 0);
        } else continue;
    }
    free(data);
    return 1;
}