
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

void left_redirection_to_cmd(tsh_memory *memory,char * command){ //does for example: "cat < fic > fic2" --> "cat fic" or "ls rep > fic" --> "ls rep"
    char copy = strdup(command);
    char cmd[512];
    char *tok;
    if((tok = strtok(copy,"<")) != NULL){
        strncpy(cmd,tok, strlen(tok)-1); //remove the last char (= space) from tok
    }
    if((tok = strtok(NULL,">2>"))  != NULL){ //cant be null if we called this function
        strcat(cmd,tok+1);
    }
}

redirection_array* split_redirection_output(char *input){

    redirection_array *data = malloc(sizeof(redirection_array));
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
        if( (end = strstr(buf + tok_length," ")) != NULL){
            length = end - (buf + tok_length);
        }else return NULL;
        //strncpy(data->OUTPUT_NAME, buf + tok_length, length);
        fill_redir_array(data, buf + tok_length, length, 1, append);
    }
    return data;
}

int right_redirection(tsh_memory *memory, char *target, int output, int ecriture, int append){
    if(is_unix_directory(target) == 1 || target[strlen(target)-1] == '/') return -1; //target given is a directory

    copyMemory(memory,&old_memory); //saving current state of the tsh_memory

    //will allow to cd the closest directory outside the tar
    char path_outside_tar[512];
    sprintf(path_outside_tar, "%s%c", (getcwd(path_outside_tar, sizeof(path_outside_tar)), "/")); //concats directory work with a "/"
    
    char *filename = target;
    char location[512];
    getLocation(target,location);
    if(strlen(location) > 0){
        if(cd(location,memory) == -1)//path given doesn't exist
            return -1;
        filename = target + strlen(location);
    } 

    int old_stdout = dup(STDOUT_FILENO);
    if(in_a_tar(memory) == 0){ //user wants the output file outside a tar
        int fd_target = 0;
        if(append == 0) // ">" was given
            if( (fd_target = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0644) == -1)) return -1;
        else if(append == 1) // ">>" was given
            if( (fd_target = open(filename, O_CREAT | O_APPEND | O_RDWR, 0644) == -1)) return -1;

        dup2(fd_target,STDOUT_FILENO);
        
        dup2(old_stdout,STDOUT_FILENO);
        close(fd_target);
    }
    close(old_stdout);
}

int redirection(tsh_memory *memory, int fd_tar){
    //récupère la commande voulu par l'utilisateur et un éventuel redirection vers la gauche


    //récupère et met dans le tableau les redirections vers la droite voulu par l'utilisateur
    redirection_array *data = split_redirection_output(memory->comand); //line_cmd à changer

    //boucle qui fait parcours dans ARRAY.OUTPUT_NAME
    for(int i = 0; i < data->NUMBER; i++){
        if((i+1) == data->NUMBER){
            //on fait les dup, dup2 sur la sortie standard ou erreur etc..
            //execSimpleCommand(nouveau tsh avec uniquement la commande voulu par utilisateur et son arg)
            //on remet le sorties en place
        } else {
        }
    }
}