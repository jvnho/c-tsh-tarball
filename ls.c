#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "ls.h"
#include "cd.h"
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"

tsh_memory old_memory; //will be use to save/restore a memory

int is_in_array(char *string, struct ls_memory mem){ //checking if string is in ls_memory's NAME array
    for(int i = 0; i < mem.NUMBER; i++){
        if(strcmp(string, mem.NAME[i]) == 0) return 1;
    }
    return 0;
}

char file_type(char typeflag){
    if(typeflag == '0') return '-';
        return 'd';
}

void fill_info_array(struct posix_header hd, struct ls_memory *mem){ //filling ls_memory's INFO array
    int filesize = 0;
    sscanf(hd.size,"%o",&filesize);
    char c[(strlen(hd.uname)+strlen(hd.gname)+strlen(hd.size)+12) * sizeof(char)];
    sprintf(c, "%c%s %s %s %d", file_type(hd.typeflag), octal_to_string(hd.mode), hd.uname, hd.gname, filesize);
    strcpy(mem->INFO[mem->NUMBER], c);
}

int is_array_empty(struct ls_memory mem){
    return mem.NUMBER == 0;
}

void print_ls_l(struct ls_memory mem){ //if option -l was given by user
    for(int i = 0; i < mem.NUMBER; i++){
        write(1, mem.INFO[i], strlen(mem.INFO[i]));
        write(1, " ", 1);
        write(1, mem.NAME[i], strlen(mem.NAME[i]));
        write(1, "\n", 1);
    }
}

void print_ls(struct ls_memory mem){//if no option was given
    for(int i = 0; i < mem.NUMBER; i++){
        write(1, mem.NAME[i], strlen(mem.NAME[i]));
        write(1," ", 1);
    }
    write(1,"\n",1);
}

void print_ls_to_STROUT(int arg_l, struct ls_memory mem){
    if(arg_l == 0) print_ls(mem);
    else print_ls_l(mem);
}

void print_filename_stdout(char *name){
    write(1, name, strlen(name));
    write(1, "\n",1);
}

void clear_struct(struct ls_memory *mem){
    mem-> NUMBER = 0;
    memset(mem->NAME, 0, sizeof(mem->NAME));
    memset(mem->INFO, 0, sizeof(mem->INFO));
}

//full_path eiter the FAKE_PATH or the concatenation of FAKE_PATH with a file we looking for.
int ls_in_tar(int fd, char* full_path, int arg_l, char type){
    lseek(fd, 0, SEEK_SET);
    struct posix_header hd;

    struct ls_memory mem;
    clear_struct(&mem);

    int len_path = strlen(full_path), fic_found = 0;

    while(read(fd, &hd, BLOCKSIZE) > 0){ //reading the entire tarball

        int i = len_path;

        if(type == 'f' && strcmp(hd.name, full_path)==0 && hd.typeflag!= 5){
            if(type == 'f'){ // treating a file
                while(hd.name[i-1] != '/' && i > 0)
                    i--;
                print_filename_stdout(hd.name+i);
                return 1;
            }
        }
        if( type != 'f' && strncmp(hd.name, full_path, len_path)==0 && strcmp(hd.name, full_path)!=0){
            char CUT_PATH[255];//CUT_PATH = file or directory name cut from its path
            int taille_nom = 0;
            while(hd.name[i] != '\0' && hd.name[i] != '/' ){
                i++; taille_nom++;
            }
            strncpy(CUT_PATH, hd.name+len_path, taille_nom); //"cutting" the name from its path
            CUT_PATH[taille_nom++] = '\0';

            if( is_in_array(CUT_PATH, mem) == 0){ //checking if the file is not is the array (to not print in more than once)
                if(arg_l == 1) fill_info_array(hd, &mem);//filling FILE_INFO's array if -l argument is given
                strcpy(mem.NAME[mem.NUMBER], CUT_PATH);//copying CUT_PATH(i.e file/rep name to ARRAY)
                mem.NUMBER++;
                fic_found++;
            }
        }
        //allow to jump to the next hd block
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        lseek(fd,512*nb_bloc_fichier, SEEK_CUR);
    }
    if(is_array_empty(mem) == 0) print_ls_to_STROUT(arg_l, mem); //there is at least one thing to display
    return fic_found;
}

//making an array for execvp
char** execvp_array(char *dir, char option[50][50], int nb_option){
    char **ret;
    assert((ret = malloc(sizeof(char*) * (nb_option+3))) != NULL );
    assert((ret[0] = malloc(sizeof(char)*2)) != NULL);
    strcpy(ret[0], "ls");
    int index = 1;
    for(int i = 0; i < nb_option; i++){
        assert((ret[index] = malloc(sizeof(char)*strlen(option[i]))) != NULL);
        strcpy(ret[index], option[i]);
        index++;
    }
    if(dir != NULL){
        assert((ret[index] = malloc(sizeof(char)*strlen(dir))) != NULL);
        strcpy(ret[index], dir);
        index++;
    }
    ret[index] = NULL;
    return ret;
}

int option_l_present(char option[50][50], int nb_option){
    for(int i = 0; i < nb_option; i++)
        if(strcmp(option[i], "-l") == 0)
            return 1;
    return 0;
}

void exec_ls(char **args){
    int r = fork();
    if(r == 0) execvp("ls", args);
    else wait(NULL);
}

void restoreLastState(tsh_memory *memory){
    copyMemory(&old_memory, memory); //restoring the last state of the memory
    char *destination = malloc(strlen(memory->REAL_PATH));
    strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
    cd(destination,memory); //cd-ing back to where we were
    free(destination);
}

void do_ls(tsh_memory *memory, char *dir, char option[50][50], int nb_option, int l_opt){
    copyMemory(memory,&old_memory); //saving current state of the tsh_memory

    if(dir[strlen(dir)-1] == '/'  || is_unix_directory(dir) == 1){ //we are sure the user entered a directory to ls (@string_traitement.c)
        if(cd(dir, memory) > -1){ //cd-ing to the directory location (if it exists)
            if(in_a_tar(memory) == 1) ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt, 'd');
            else exec_ls(execvp_array(NULL,option,nb_option));
        }
    } else { // in this case, we don't know if the user wants to ls a directory or a file
        char location[512], *dirToVisit = dir;
        getLocation(dir,location); //@string_traitement.c for details

        //in this case, user wants to ls a .tar file so we cd in it and run ls
        if(strlen(location) == 0 && is_extension_tar(dirToVisit) == 1){ //@string_traitement.c
            cd(dirToVisit, memory);
            ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt, 'd');
            restoreLastState(memory);
            return;
        }

        if(strlen(location) > 0){
            if(cd(location, memory) == -1) return; //path doesn't exist
        }
        dirToVisit += strlen(location);
        if(in_a_tar(memory) == 1){
            char *file_path = concate_string(memory->FAKE_PATH, dirToVisit);
            if(ls_in_tar(atoi(memory->tar_descriptor), file_path, l_opt, 'f') == 0){ //try to ls a file
                if(cd(dirToVisit, memory) > -1){ //trying to get into the dir
                    if(in_a_tar(memory) == 1) ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt, 'd');
                }
            }
        } else exec_ls(execvp_array(dirToVisit,option,nb_option));
    }
    restoreLastState(memory);
}

int ls(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option){
    int l_opt = option_l_present(option, nb_option);
    if(nb_arg == 0){
        if(in_a_tar(memory) == 1) ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt, 'd');
        else exec_ls(execvp_array(NULL,option,nb_option));
    } else {
        for(int i = 0; i < nb_arg; i++){
            do_ls(memory, args[i], option, nb_option, l_opt);
        }
    }
    return 1;
}
