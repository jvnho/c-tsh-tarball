#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "simpleCommande.h"
#include "tar.h"
#include "redirection.h"
#include "cd.h"
#include "string_traitement.h"
#include "tsh_memory.h"
#include "mkdir.h"
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
    new_cmd[strlen(new_cmd) -1] = '\0';
    strcpy(memory->comand, new_cmd);
}

void fill_redir_array(redirection_array *data, char *str, int output, int append){
    strcpy(data->OUTPUT_NAME[data->NUMBER], str);
    data->STD[data->NUMBER] = output;
    data->APPEND[data->NUMBER] = append;
    data->NUMBER++;
}

struct redirection_array* associate_redirection(char *cmd){
    struct redirection_array *data = malloc(sizeof(redirection_array)); // check @redirection.h
    char *tok = strtok(cmd," ");
    while(tok != NULL){
        if(strstr(tok, "2>") != NULL){
            if(strcmp(tok, "2>&1") == 0){
                tok = strtok(NULL, " "); //next token
                fill_redir_array(data, tok, 3, 0);
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

char* cmd_output_to_pipe(tsh_memory *memory, int std){
    int unused_std;
    if(std == 1) unused_std = STDERR_FILENO;
    else unused_std = STDOUT_FILENO;

    char *output;
    assert((output = malloc(sizeof(char) * 512)) != NULL);
    int fd_null;
    if((fd_null = open("/dev/null", O_WRONLY)) == -1) exit(-1);

    int fd_pipe[2];
    pipe(fd_pipe);
    int pid = fork();
    if(pid == -1){
        exit(-1);
    } else if(pid == 0){ //child proccess
        close(fd_pipe[0]);
        dup2(fd_pipe[1], std);
        dup2(fd_null, unused_std);
        execSimpleCommande(memory);
        //dup2(old_std, std); nécessaire ou pas ???
        close(fd_pipe[1]);
        exit(0);
    } else { //parent proccess
        close(fd_pipe[1]);
        int read_size = 0, read_size_total = 0;
        char buf[512];
        while( (read_size = read(fd_pipe[0], buf, 512)) > 0){
            read_size_total += read_size;
            if(read_size > sizeof(output)) (assert((realloc(output, sizeof(output)+512)) != NULL));
            strcat(output,buf);
        }
        close(fd_pipe[0]);
    }
    close(fd_null);
    return output;
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

void new_file_in_tar(int fd_tar, struct posix_header *new_header, char *content, int nb_new_block){
    put_at_the_first_null(fd_tar);
    write(fd_tar, new_header, 512); //creating a new header
    write(fd_tar, content, nb_new_block*512); //content
    //adding zero block byte (+1 for the header)
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    for(int i = 0; i < nb_new_block +1; i++)
        write(fd_tar, end_bloc, 512);
}

void redirection_in_tar(tsh_memory *memory, char *redir_name, int std, int *out_written, int *err_written, char *out, char *err, int append){
    char content[strlen(out) + strlen(err) + 1];
    memset(content, 0, strlen(out) + strlen(err));
    int fd_tar = atoi(memory->tar_descriptor);
    switch(std){
        case 3:
            sprintf(content, "%s\n%s", out, err);
            *out_written = 1, *err_written = 1;
            break;
        case 2:
            strcat(content, err);
            *err_written = 1;
            break;
        case 1:
            strcat(content, out);
            *out_written = 1;
            break;
    }
    char *path_to_file = concate_string(memory->FAKE_PATH, redir_name);
    struct posix_header *new_header = create_header(path_to_file, '1');//new header file
    int block_to_add = ceil(strlen(content)/512);

    int block_to_move = 0;
    off_t file_offset = file_location(fd_tar, path_to_file, &block_to_move); //file location in tar (if exists) and its number of content blocks

    if(file_offset == -1){ //file doesn't exist in the tar
        new_file_in_tar(fd_tar, new_header, content, block_to_add);
    } else { //user wants to append redirection (double arrows)
        char old_content[512 * block_to_move];
        lseek(fd_tar, file_offset+512,SEEK_CUR);
        if(append ==1) read(fd_tar, old_content, 512*block_to_move);
        rm_in_tar(fd_tar, path_to_file, 0, 1);
        new_file_in_tar(fd_tar, new_header, content, block_to_add);
    }
    free(new_header);
    free(path_to_file);
}

int redirection(tsh_memory *memory){
    struct redirection_array *data = associate_redirection(strdup(memory->comand)); //associating every redirection to the standard output/error interested by user
    convert_to_simple_cmd(memory); //converting command line entered by user to make it readable by the programm

    //executing command while redirecting stdout and stderr
    char *out = cmd_output_to_pipe(memory,1); //what stdout received
    char *err = cmd_output_to_pipe(memory,2); //what stderr received

    int out_written = 0, err_written = 0; //allow us to know if we wrote function output in a redirection

    for(int i = 0; i < data->NUMBER; i++){
        char *redir_name = data->OUTPUT_NAME[i];
        if(is_unix_directory(redir_name) == 1 || redir_name[strlen(redir_name)-1] == '/') return -1; //redirection given is a directory

        copyMemory(memory, &old_memory);//keep the curent state of the programm

        char location[512];
        memset(location, 0, 512);

        getLocation(redir_name, location);
        if(strlen(location) > 0) {//if there is an extra path cd to that path
            if(cd(location, memory)== -1) return -1; //path doesn't exist
            redir_name = redir_name + strlen(location);
        }
        
        int append = data->APPEND[i], std = data->STD[i];
        if(in_a_tar(memory) == 0) redirection_out_tar(redir_name, std, &out_written, &err_written, out, err, append); //redirection file is outside the tar
        else redirection_in_tar(memory, redir_name, std, &out_written, &err_written, out, err, append); //redirection file is inside a tar

        restoreLastState(old_memory, memory);
    }
    if(err_written == 0){
        write(2, err, strlen(err));
        write(2, "\n", 1);
    }
    if(out_written == 0) {
        write(1, out, strlen(err));
        write(1, "\n", 1);
    }
    free(out);
    free(err);
    return 1;
}