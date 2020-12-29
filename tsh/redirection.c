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
    memset(new_cmd, 0, sizeof(new_cmd));

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

int fill_redir_array(tsh_memory *src_memory, redirection_array *data, char *target, int output, int append){
    if(is_unix_directory(target) == 1 || target[strlen(target)-1] == '/') return -1; //redirection given is a directory so we abort

    tsh_memory target_mem;
    copyMemory(src_memory, &target_mem);//Copy the curent state of the programm

    char *redir_name = target, location[512];
    getLocation(redir_name, location);
    int lenLocation = strlen(location);
    if(lenLocation > 0)
    {
        if(cd(location, &target_mem) == -1) return -1; //if one of the redirection is incorrect we abort
        redir_name += strlen(location);
    }
    else if(lenLocation == 0 && strstr(redir_name, ".tar")){
        if(cd(target, &target_mem) == -1) return -1; //same
    }

    //Be careful and notice src_memory and target_mem
    int index = data->NUMBER;
    if(in_a_tar(src_memory) == 0) //outside tar to (somewhere...)
    {
        if(in_a_tar(&target_mem) == 0){ //outside a tar to outside a tar
            data->IN_A_TAR[index] = 0;
            strcpy(data->REDIR_PATH[index], target);
            //champ NAME de la struct pas important dans ce cas-là
        }
        else //outside a tar to inside a tar
        {
            data->IN_A_TAR[index] = 1;
            strcpy(data->NAME[index], redir_name); //name of the file
            char *cur_full_path = getPath(&target_mem);
            cur_full_path[strlen(cur_full_path)-2] = '\0'; //removing'$'
            strcpy(data->REDIR_PATH[index],cur_full_path); //where the file will be relocated
        }
    }
    else //inside a tar to (somewhere...)
    { 
        if(in_a_tar(&target_mem) == 1) //inside a tar to a inside a tar
        {
            //MARCHE MAIS PAS A LA RACINE DU TAR (obligé de donner un chemin dans un tar)
            data->IN_A_TAR[index] = 1;

            char location_src[512]; //location of the created file which is just outside the open tar
            getcwd(location_src, 512);
            strcat(location_src, "/");
            //voir si len location == 0 et que pas de slash à la  fin pour la racine
            strcat(location_src, redir_name);
            
            strcpy(data->NAME[index], location_src); //location of the source file donner le chemin absolu juste en dehors du tar

            char redir_path[512];
            memset(redir_path, 0, 512);
            strncpy(redir_path, target_mem.REAL_PATH, strlen(target_mem.REAL_PATH)-2);
            strcpy(data->REDIR_PATH[index], location);
        } 
        else // inside a tar to outside a tar
        {   
            printf("tar to ");
            data->IN_A_TAR[index] = 0;
            char path[512];
            strncpy(path,target_mem.REAL_PATH, strlen(target_mem.REAL_PATH)-2);

            char pathBuffer[512];
            concatenation(path, redir_name,pathBuffer);
            strcpy(data->REDIR_PATH[index], pathBuffer);
        }
    }
    data->STD[index] = output;
    data->APPEND[index] = append;
    data->NUMBER++;
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

int redirection(tsh_memory *memory){
    struct redirection_array *data = associate_redirection(memory, strdup(memory->comand)); 
    if(data == NULL || is_redirection_valid(data) == -1)
    {
        if(data != NULL) free(data);
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
        }else if(data->STD[i] == 2) //User wants a "2>" redirection (maybe 2>> if append == 1)
        { 
            if(append == 1) 
                fd_fic_err = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else 
                fd_fic_err = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_err != -1);
        }else //User wants a ">" redirection (maybe 2>> if append == 1)
        { 
            if(append == 1) 
                fd_fic_out = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else 
                fd_fic_out = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            assert(fd_fic_out != -1);
        }
        if(fd_fic_mix != 0){ //If we opened a file to receive 2>&1
            dup2(fd_fic_mix, STDOUT_FILENO);
            dup2(fd_fic_mix, STDERR_FILENO);
        } 
        else 
        {
            if(fd_fic_out != 0){ //If we opened a file to receive >
                if(dup2(fd_fic_out, STDOUT_FILENO) == -1) return -1;
            }
            if(fd_fic_err != 0){ //If we opened a file to receive 2>
                if(dup2(fd_fic_err, STDERR_FILENO) == -1) return -1;
            }
        }
    }
    
    execSimpleCommande(memory); //output and/or errors will be written in open file

    //restoring stdout and stderr
    dup2(old_stdout, STDOUT_FILENO);
    dup2(old_stderr, STDERR_FILENO);

    //close eventual open files
    if(fd_fic_out != 0) close(fd_fic_out);
    if(fd_fic_err != 0) close(fd_fic_err);
    if(fd_fic_mix != 0) close(fd_fic_mix);

    //Procedure to move the file into the tar in needed
   for(int i = 0; i < data->NUMBER; i++) 
    { 
        if(data->IN_A_TAR[i] == 1){ //redirection file was supposed to be in a tar so we move it
            do_mv(memory, data->NAME[i],data->REDIR_PATH[i], NULL, 0);
        }
    }
    free(data);
    return 1;
}