
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

redirection_array* split_redirection_output(char *input){ //does for example: from "ls > fic >> fic2" to ["fic";"fic2"]

    redirection_array *data = malloc(sizeof(redirection_array));
    memset(&data, 0, sizeof(data)); //to avoid any segmentation fault

    char *copy = strdup(input), *tok = strtok(copy, ">>>2>2>>2>&1");
    while(tok != NULL){
        char *output_name = tok+1, *end;
        if( (end = (strstr(output_name," "))) == NULL){
            end = strstr(output_name,"\0");//garanteed not null
        }
        int length = end - output_name; //name of the redirection

        char redirection[512];
        if(length > 0){ //on prend jusqu'à un certain point avant la fin de la chaîne
            strncpy(redirection, output_name, length);
            redirection[length] = '\0';
        } else { //on prend jusqu'à la fin de la chapîne de charactere
            strcpy(redirection, output_name);
            redirection[strlen(output_name)] = '\0';
        }
        strcpy(data->OUTPUT_NAME[data->NUMBER], redirection);
        data->NUMBER++;
        data->APPEND[data->NUMBER] = 0;
        
        tok = strtok(NULL, ">>>2>2>>2>&1");
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