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

void fill_redir_array(redirection_array *data, char *str, int output, int append){
    strcpy(data->OUTPUT_NAME[data->NUMBER], str);
    data->STD[data->NUMBER] = output;
    data->APPEND[data->NUMBER] = append;
    data->NUMBER++;
}

struct redirection_array* associate_redirection(char *cmd){ //associating every redirection to the standard output/error interested by user
    struct redirection_array *data = malloc(sizeof(redirection_array)); // check @redirection.h
    char *tok = strtok(cmd," ");
    while(tok != NULL){
        if(strstr(tok, "2>") != NULL){
            if(strcmp(tok, "2>&1") == 0){
                tok = strtok(NULL, " "); //next token
                fill_redir_array(data, tok, 3, 1);
                return data; //we consider user cant use 2>&1 with other redirection (no sense)
            } else {
                int append;
                if((tok+2)[0] == '>') append = 1; //2>>
                else append = 0;
                tok = strtok(NULL, " "); //next token
                fill_redir_array(data, tok, 2,append);
            }
        } else if(strstr(tok, ">") != NULL){
            int append;
            if( (tok+1)[0] == '>') append = 1; //>>
            else append = 0;
            tok = strtok(NULL, " "); //next token
            fill_redir_array(data, tok, 1,append);
        } 
        else tok = strtok(NULL," ");
    }
    return data;
}

//function that writes the output (stdout and/or stderr) of a command in a file outside a tar
void redirection_out_tar(char *redir_name, int std, int *out_written, int *err_written, char *out, char *err, int append){
    int fd_file;
    if(append == 1) fd_file = open(redir_name, O_RDWR | O_CREAT | O_APPEND, 0644);
    else fd_file = open(redir_name, O_RDWR | O_CREAT | O_TRUNC, 0644);
    switch(std){
        case 3: // 2>&1
            write(fd_file, out, strlen(out));
            write(fd_file, err, strlen(err));
            *out_written = 1, *err_written = 1;
            return;
        case 2: //2> (or 2>>)
            write(fd_file, err, strlen(err));
            *err_written = 1;
            break;
        case 1: //> (or >>)
            write(fd_file, out, strlen(out));
            *out_written = 1;
            break;
    }
}

int redirection(tsh_memory *memory){
    struct redirection_array *data = associate_redirection(strdup(memory->comand)); 
    convert_to_simple_cmd(memory); //converting command line entered by user to make it readable by the programm

    print_data(data);
    printf("new cmd %s\n", memory->comand);
    
    copyMemory(memory, &old_memory);//keep the curent state of the programm

    int fd_fic_out = 0, fd_fic_err = 0, fd_fic_mix = 0;//respectively file descriptor than will receive redirection from >, 2>, 2>&1
    int old_stdout = dup(STDOUT_FILENO), old_stderr = dup(STDERR_FILENO); 

    for(int i = 0; i < data->NUMBER; i++){ //there is at most TWO redirection entered
        char *redir_name = data->OUTPUT_NAME[i];
        printf("NOM %s\n\n", redir_name);
        if(is_unix_directory(redir_name) == 1 || redir_name[strlen(redir_name)-1] == '/') return -1; //redirection given is a directory

        char location[512];
        getLocation(redir_name, location);
        if(strlen(location) > 0){
            redir_name += strlen(location);
        }

        int append = data->APPEND[i];
        if(data->STD[i] == 3){ //user wants a "2>&1" redirection 
            fd_fic_mix = open(redir_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
            assert(fd_fic_mix != -1);
            printf("REDIRECT VERS 2>&1\n");
        } 
        else if(data->STD[i] == 2){ //user wants a "2>" redirection (maybe 2>> if append == 1)
            if(append == 1) fd_fic_err = open(redir_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_err = open(redir_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_err != -1);
            printf("REDIRECT VERS 2>\n");
        } 
        else { //user wants a ">" redirection (maybe 2>> if append == 1)
            if(append == 1) fd_fic_out = open(redir_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else fd_fic_out = open(redir_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_out != -1);
            printf("REDIRECT VERS >\n");
        }
        if(fd_fic_mix != 0){ //if we opened a file to receive 2>&1
            dup2(fd_fic_mix, STDOUT_FILENO);
            dup2(fd_fic_mix, STDERR_FILENO);
        } else {
            if(fd_fic_out != 0){
                printf("allo\n");
                dup2(fd_fic_out, STDOUT_FILENO);
            }
            if(fd_fic_err != 0){
                dup2(fd_fic_err, STDERR_FILENO);
            }
        }
    }
    execSimpleCommande(memory);
    //restoring stdout and stderr
    dup2(old_stdout, STDOUT_FILENO);
    dup2(old_stderr, STDERR_FILENO);
    restoreLastState(old_memory, memory);
    return 1;
}