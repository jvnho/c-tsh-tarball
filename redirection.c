#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>

#include "simpleCommande.h"
#include "tar.h"
#include "redirection.h"
#include "cd.h"
#include "string_traitement.h"
#include "tsh_memory.h"
#include "rm.h"
#include "exec_funcs.h"

tsh_memory old_memory; //will be use to save/restore a memory

void print_data(redirection_array *array){
    printf("%d\n", array->NUMBER);
    for(int i = 0; i < array->NUMBER; i++){
        printf("name %s\n", array->OUTPUT_NAME[i]);
        printf("std %d\n", array->STD[i]);
        printf("append %d\n", array->APPEND[i]);
    }
}

off_t file_location(int fd, char *needle, int *content_blocks){ //locate the file in the tar and returns its offset and num of content blocks or if not found -1
    struct posix_header hd;
    while(read(fd, &hd,512) > 0){
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;

        if(strcmp(hd.name, hd.name) == 0 && hd.typeflag != '5'){
            *content_blocks = nb_bloc_fichier;
            return lseek(fd, -512, SEEK_CUR);
        } 
        lseek(fd,512*nb_bloc_fichier, SEEK_CUR);
    }
    return -1; //file not found
}

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
    char *redir_name = str, location[512];
    memset(location,0, 512);
    getLocation(redir_name, location);
    if(strlen(location) > 0){
        copyMemory(memory, &old_memory);//Keeps the curent state of the programm
        if(cd(location, memory) == -1){//Path given as redirection doesn't exist
            return -1; //one of the redirection is incorrect we abort
        }
        redir_name += strlen(location);
    }
    if(in_a_tar(memory) == 1){
        data->IN_A_TAR[data->NUMBER] = 1;
    } else {
        data->IN_A_TAR[data->NUMBER] = 0;
    }
    restoreLastState(old_memory, memory);
    strcpy(data->OUTPUT_NAME[data->NUMBER], str);
    data->STD[data->NUMBER] = output;
    data->APPEND[data->NUMBER] = append;
    data->NUMBER++;
    return ;
}

struct redirection_array* associate_redirection(tsh_memory *memory, char *cmd){ //associating every redirection to the standard output/error interested by user
    struct redirection_array *data = malloc(sizeof(redirection_array)); // check @redirection.h
    char *tok = strtok(cmd," ");
    while(tok != NULL){
        if(strstr(tok, "2>") != NULL){
            if(strcmp(tok, "2>&1") == 0){
                tok = strtok(NULL, " "); //next token
                fill_redir_array(memory, data, tok, 3, 1);
                return data; //we consider user cant use 2>&1 with other redirection (no sense)
            } else {
                int append;
                if((tok+2)[0] == '>') append = 1; //2>>
                else append = 0;
                tok = strtok(NULL, " "); //next token
                fill_redir_array(memory, data, tok, 2,append);
            }
        } else if(strstr(tok, ">") != NULL){
            int append;
            if( (tok+1)[0] == '>') append = 1; //>>
            else append = 0;
            tok = strtok(NULL, " "); //next token
            fill_redir_array(memory, data, tok, 1,append);
        } 
        else tok = strtok(NULL," ");
    }
    return data;
}

int delete_system_file(char *path){
    int pid = fork();
    if(pid == 0){ //child
        execlp("rm", "rm", path, NULL);
    } else { //parent
        waitpid(pid, NULL, 0);
    }
}

int redirection(tsh_memory *memory){
    struct redirection_array *data = associate_redirection(memory, strdup(memory->comand)); 
    if(data->NUMBER > 2) return -1;
    convert_to_simple_cmd(memory); //converting command line entered by user to make it readable by the programm

    int fd_fic_out = 0, fd_fic_err = 0, fd_fic_mix = 0;//respectively file descriptor than will receive redirection from >, 2>, 2>&1
    int old_stdout = dup(STDOUT_FILENO), old_stderr = dup(STDERR_FILENO); 

    for(int i = 0; i < data->NUMBER; i++){ //going through this loop at most 2 times 
        char *redir_name = data->OUTPUT_NAME[i];
        if(is_unix_directory(redir_name) == 1 || redir_name[strlen(redir_name)-1] == '/') return -1; //redirection given is a directory

        char location[512];
        memset(location,0, 512);
        getLocation(redir_name, location);
        if(strlen(location) > 0){
            if(cd(location, memory) == -1){//Path given as redirection doesn't exist
                return -1; //one of the redirection is incorrect we abort
            }
            copyMemory(memory, &old_memory);//Keeps the curent state of the programm
            redir_name += strlen(location);
        }

        int append = data->APPEND[i];
        if(data->STD[i] == 3){ //user wants a "2>&1" redirection 
            fd_fic_mix = open(redir_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
            assert(fd_fic_mix != -1);
        } 
        else if(data->STD[i] == 2){ //user wants a "2>" redirection (maybe 2>> if append == 1)
            if(append == 1) fd_fic_err = open(redir_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_err = open(redir_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_err != -1);
        } 
        else { //user wants a ">" redirection (maybe 2>> if append == 1)
            if(append == 1) fd_fic_out = open(redir_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_out = open(redir_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_out != -1);
        }
        if(fd_fic_mix != 0){ //if we opened a file to receive 2>&1
            dup2(fd_fic_mix, STDOUT_FILENO);
            dup2(fd_fic_mix, STDERR_FILENO);
        } else {
            if(fd_fic_out != 0){ //if we opened a file to receive >
                dup2(fd_fic_out, STDOUT_FILENO);
            }
            if(fd_fic_err != 0){ //if we opened a file to receive 2>
                dup2(fd_fic_err, STDERR_FILENO);
            }
        }
    }
    execSimpleCommande(memory); //output and/or errors will be written in open file
    //close(fd_fic_out);
    //close(fd_fic_err);
    //close(fd_fic_mix);

    //restoring stdout and stderr
    dup2(old_stdout, STDOUT_FILENO);
    dup2(old_stderr, STDERR_FILENO);

    for(int i = 0; i < data->NUMBER; i++){ //Now we move file to the tar if wanted by user

    }
    //restoreLastState(old_memory, memory);
    return 1;
}