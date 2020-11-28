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
        write(1, strcat(mem.INFO[i]," "), strlen(mem.INFO[i])+1);
        write(1, strcat(mem.NAME[i]," \n"), strlen(mem.NAME[i])+2);
    }
}

void print_ls(struct ls_memory mem){//if no option was given
    for(int i = 0; i < mem.NUMBER; i++)
        write(1, strcat(mem.NAME[i]," "), strlen(mem.NAME[i])+2);
    write(1,"\n",1);
}

void print_ls_to_STROUT(int arg_l, struct ls_memory mem){
    if(arg_l == 0) print_ls(mem);
    else print_ls_l(mem);
}

void clear_struct(struct ls_memory *mem){
    mem-> NUMBER = 0;
    memset(mem->NAME, 0, sizeof(mem->NAME));
    memset(mem->INFO, 0, sizeof(mem->INFO));
}

//full_path is the concatenation of tsh_memory's FAKE_PATH + a directory or a file
void ls_in_tar(int fd, char* full_path, int arg_l){
    lseek(fd, 0, SEEK_SET);
    struct posix_header hd;
    struct ls_memory mem;
    int len_path = strlen(full_path);

    while(read(fd, &hd, BLOCKSIZE) > 0){ //reading the entire tarball
        //checking if the current file/repository belongs to the given full_path and if it's not itself (to not print in)
        if( (strncmp(hd.name, full_path, len_path) == 0 && strcmp(hd.name, full_path) != 0) //notice "strncmp" and "strcmp"
        || (strcmp(hd.name, full_path) == 0 && hd.typeflag != '5')){

            int i = len_path, taille_nom = 0;
            while(hd.name[i] != '\0' && hd.name[i] != '/' ){
                i++; taille_nom++;
            }
            char CUT_PATH[255];//CUT_PATH = file or directory name cut from its path
            strncpy(CUT_PATH, hd.name+len_path, taille_nom); //"cutting" the name from its path
            CUT_PATH[taille_nom++] = '\0';

            if( is_in_array(CUT_PATH, mem) == 0){ //checking if the file is not is the array (to not print in more than once)
                if(arg_l == 1) fill_info_array(hd, &mem);//filling FILE_INFO's array if -l argument is given
                strcpy(mem.NAME[ (mem.NUMBER)++ ], CUT_PATH);//copying CUT_PATH(i.e file/rep name to ARRAY)
            }
        }
        //allow to jump to the next hd block
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        lseek(fd,512*nb_bloc_fichier, SEEK_CUR);
    }
    if(is_array_empty(mem) == 0) print_ls_to_STROUT(arg_l, mem); //there is at least one thing to display
    clear_struct(&mem);
}

//making an array for execvp
char** execvp_array(char option[50][50], int nb_option){
    char **ret;
    assert((ret = malloc(sizeof(char*) * (nb_option+2))) != NULL );
    assert((ret[0] = malloc(sizeof(char)*2)) != NULL);
    strcpy(ret[0], "ls");
    for(int i = 1; i < nb_option+1; i++){
        assert((ret[i] = malloc(sizeof(char)*strlen(option[i-1]))) != NULL);
        strcpy(ret[i], option[i-1]);
    }
    ret[nb_option+1] = NULL;
    return ret;
}

int option_l_present(char option[50][50], int nb_option){
    for(int i = 0; i < nb_option; i++)
        if(strcmp(option[i], "-l") == 0)
            return 1;
    return 0;
}

void exec_ls(char **option){
    int r = fork();
    if(r == 0) execvp("ls", option);
    else wait(NULL);
}

void do_ls(tsh_memory *memory, char *dir, char option[50][50], int nb_option, int l_opt){
    copyMemory(memory,&old_memory); //saving current state of the tsh_memory

    int ls_a_dir = 0;
    char location[512], *dirToVisit;
    getLocation(dir,location); //@string_traitement.c for details
    dirToVisit = dir + strlen(location);

    if(dir[strlen(dir)-1] == '/'  || is_unix_directory(dirToVisit) == 1){ //we are sure the user entered a directory to ls
        printf("%s\n", dirToVisit);
        ls_a_dir = 1;
        if(cd(dir, memory) > -1){ //cd-ing to the directory location (if it exists)
            if(in_a_tar(memory) == 1) ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt);
            else exec_ls(execvp_array(option,nb_option));
        }
    } else { //we don't know if the user wants to ls a directory or a file

        // if(cd(location, memory) > -1){ //cd-ing to the directory location (if it exists)
        //     //but has the user entered a directory or a file ?
        //     if(in_a_tar(memory) == 1){
        //         if(ls_a_dir == 1){
        //             ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt);
        //             return;
        //         } else if(is_unix_directory(dirToVisit) == 0){ //@string_traitement.c for details
        //                 cd(dirToVisit+strlen(location), memory);
        //                 ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt);
        //         } else {
        //             ls_in_tar(atoi(memory->tar_descriptor), concate_string(memory->FAKE_PATH, dirToVisit+strlen(location)), l_opt);
        //         }
        //     }
        //     else exec_ls(execvp_array(option,nb_option));
        // }
    }
    copyMemory(&old_memory, memory); //restoring the last state of the memory
    char *destination = malloc(strlen(memory->REAL_PATH));
    strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
    cd(destination,memory); //cd-ing back to where we were
}

int ls(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option){
    int l_opt = option_l_present(option, nb_option);
    for(int i = 0; i < nb_arg; i++){
        do_ls(memory, args[i], option, nb_option, l_opt);
    }
    return 1;
}
