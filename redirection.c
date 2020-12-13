#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tar.h"
#include "redirection.h"
#include "simpleCommande.h"
#include "cd.h"
#include "string_traitement.h"
#include "tsh_memory.h"
#include "mkdir.h"

tsh_memory old_memory; //will be use to save/restore a memory

void fill_redir_array(redirection_array *data, char *str, int length, int output, int append){
    strncpy(data->OUTPUT_NAME, str, length);
    data->OUTPUT[data->NUMBER] = output;
    data->APPEND[data->NUMBER] = append;
    data->NUMBER++;
}

void remove_redir_from_cmd(tsh_memory *memory){ //removes redirection symbol from the tsh_memory "comand"
    char *cmd = memory->comand; //command line entered by user
    char new_cmd[512];
    new_cmd[0] = '\0';
    
    char *start, *end, *right;
    int gap, length;
    if(((start = strstr(cmd, "<")) != NULL)){ //user entered for ex: "cat < fic fic2..."
        length = start - cmd;
        strncpy(new_cmd, cmd, length);
        right =  cmd + strlen(new_cmd) + 2;
    } else if(((start = strstr(cmd, " ")) != NULL)){ //user entered instead "cat fic fic2..."
        length = start - cmd;
        strncpy(new_cmd, cmd, length);
        strcat(new_cmd, " ");
        right = cmd + strlen(new_cmd);
    }
    if(((end = strstr(right, "2>")) != NULL) || ((end = strstr(right, ">")) != NULL)){ //splitting args from redirection "fic fic2 > fic3" --> "fic fic2"
        length = end - right;
    }
    strncat(new_cmd, right, length);
    strcpy(memory->comand, new_cmd); // new_cmd is "cat fic fic2"
}

struct redirection_array* split_redirection_output(char *input){
    struct redirection_array *data = malloc(sizeof(redirection_array));
    memset(data, 0, sizeof(data));//to avoid any segmentation fault

    char *buf, *end;
    int length = 0, tok_length = 0;
    char name[512];
    
    if((buf = strstr(input,"2>&1")) != NULL){ //if 2>&1 was given

        tok_length = strlen("2>&1 ");
        if((end = strstr(buf + tok_length," ")) != NULL){ //can't be NULL the user must entered somewhere to redirect
            length = end - (buf + tok_length);
        } else return NULL;
        //strncpy(data->OUTPUT_NAME, buf+5, length);
        fill_redir_array(data, buf + tok_length, length, 3, 0);
        return data; //we consider user didn't entered any other output redirection if 2>&1 was given
    } 

    int append = 0;

    if((buf = strstr(input, "2>") ) != NULL){ //stderr
        tok_length = strlen("2> ");
        if((buf+2)[0] == '>'){ //enter gave double arrows (append)
            append = 1;
            tok_length++;
        } else {
            append = 0;
        }
        if( (end = strstr(buf + tok_length," ")) != NULL){
            length = end - (buf + tok_length);
        } else return NULL;
        //strncpy(data->OUTPUT_NAME, buf + tok_length, length);
        fill_redir_array(data, buf + tok_length, length, 2, append);
    }
    if((buf = strstr(input, ">") ) != NULL){ //stdout
        tok_length = strlen("> ");
        if((buf+1)[0] == '>'){ //enter gave double arrows (append)
            append = 1;
            tok_length++;
        } else { 
            append = 0;
        }
        if((end = strstr(buf + tok_length," ")) != NULL){
            length = end - (buf + tok_length);
        }else return NULL;
        //strncpy(data->OUTPUT_NAME, buf + tok_length, length);
        fill_redir_array(data, buf + tok_length, length, 1, append);
    }
    return data;
}

char* cmd_output_to_pipe(tsh_memory *memory, int std){
    int old_std;
    if(std == 1)old_std = dup(STDOUT_FILENO);
    else old_std = dup(STDERR_FILENO);//2

    char *output;
    assert((output = malloc(sizeof(char) * 512)) != NULL);
    int fd_pipe[2];

    pipe(fd_pipe);
    int pid = fork();
    if(pid == -1){
        exit(-1);
    } else if(pid == 0){ //child proccess
        close(fd_pipe[0]);
        dup2(fd_pipe[1], std);
        execSimpleCommande(memory);
        dup2(old_std, std);
        close(fd_pipe[1]);
        exit(0);
    } else { //parent proccess
        close(fd_pipe[1]);
        int read_size = 0;
        char buf[512];
        while(read(fd_pipe[0], buf, 512) > 0){
            read_size += strlen(buf);
            if(read_size > strlen(output)) (assert(realloc(output, sizeof(output)*2)) != NULL);
            strcat(output,buf);
        }
        close(fd_pipe[0]);
    }
}

int redirection(tsh_memory *memory){
    remove_redir_from_csmd(memory); //récupère la commande voulu par l'utilisateur et un éventuel redirection vers la gauche

    struct redirection_array *data = split_redirection_output(memory->comand); //récupère et met dans le tableau les redirections voulues par l'utilisateur

    char *out = cmd_output_to_pipe(memory,1); //exec command while redirecting stdout
    char *err = cmd_output_to_pipe(memory,2); //same but redirecting stderr

    int out_written = 0, err_written = 0;

    for(int i = 0; i < data->NUMBER; i++){
        char *redir_name = data->OUTPUT_NAME[i];
        if(is_unix_directory(redir_name) == 1 || redir_name[strlen(redir_name)-1] == '/') return -1; //redirection given is a directory

        copyMemory(memory, &old_memory);//keep the curent state of the programm

        char location[512];
        getLocation(redir_name, location);
        int lenLocation = strlen(location);
        if(lenLocation){//if there is an extra path cd to that path
            if(cd(location, memory)==-1) return -1; //path doesn't exist
            redir_name = redir_name + lenLocation;
        }
        int append = data->APPEND[i];
        if(in_a_tar(memory) == 0){ //redirection file is outside the tar
            int fd_file;
            if(append == 1) fd_file = open(redir_name, O_RDWR | O_CREAT | O_APPEND, 0644);
            else fd_file = open(redir_name, O_RDWR | O_CREAT | O_TRUNC, 0644);
            switch(data->OUTPUT[i]){
                case 3: // 2>&1
                    write(fd_file, out, sizeof(out));
                    write(fd_file, err, sizeof(err));
                    out_written = 1, err_written = 1;
                return 1;

                case 2: //2> (or 2>>)
                write(fd_file, err, sizeof(err));
                err_written = 1;
                break;

                case 1: //> (or >>)
                write(fd_file, out, sizeof(out));
                out_written = 1;
                break;

                default: break;
            }
        }
    }
}