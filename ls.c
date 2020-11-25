#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ls.h"
#include "cd.h"
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"

tsh_memory old_memory; //will be use to save/restore a memory

void ls_in_tar(int,char*,int);
void exec_ls(char*);

int ls(tsh_memory *memory, char args[][50], int nb_arg){
    int option = (nb_arg > 1 && (strcmp(args[1],"-l") == 0))? 1:0;

    if(nb_arg == 1 || (nb_arg == 2 && strcmp(args[1],"-l") == 0)){ //no path given
        if(in_a_tar(memory) == 1) ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, option);
        else exec_ls(NULL);
    } else {
        for(int i = (option == 1) ? 2:1; i < nb_arg; i++){
            saveMemory(memory,&old_memory);
            if(cd(args[i], memory) > -1){ //cd-ing to the directory location
                if(in_a_tar(memory) == 1)
                    ls_in_tar(atoi(memory->tar_descriptor), memory->FAKE_PATH, option);
                else
                    exec_ls(NULL);
                saveMemory(&old_memory, memory);
                char *destination = malloc(strlen(memory->REAL_PATH));
                strncpy(destination, memory->REAL_PATH, strlen(memory->REAL_PATH)-2);
                cd(destination,memory);
            }
        }
    }
    return 1;
}

void exec_ls(char *option){
    int r = fork();
    if(r == 0){ //child processus
        if(option != NULL)
            execlp("ls", "ls", option, NULL);
        else
            execlp("ls", "ls", NULL);
    } else {
        wait(NULL); //parent processus
    }
}

int is_in_array(char*, struct ls_memory);
void fill_info_array(struct posix_header, struct ls_memory*);
void print_ls_to_STROUT(int, struct ls_memory);
void clear_struct(struct ls_memory*);

//PATH is either the PATH given by the tsh_memory or its concatenation with a directory/file
void ls_in_tar(int fd, char* PATH, int arg_l){
    lseek(fd, 0, SEEK_SET);
    struct posix_header hd;
    struct ls_memory mem;
    while(read(fd, &hd, BLOCKSIZE) > 0){ //reading the entire tarball
        //checking if the current file/repository belongs to the given PATH and if it's not itself (to not print in)
        if(strncmp(hd.name, PATH, strlen(PATH)) == 0 && strcmp(PATH,hd.name) != 0){ //notice it's strncmp and not strcmp

            int i = strlen(PATH), taille_nom = 0;
            while(hd.name[i] != '\0' && hd.name[i] != '/' ){
                i++; taille_nom++;
            }
            char CUT_PATH[255];//CUT_PATH = file or directory name cut from its path
            strncpy(CUT_PATH, hd.name+strlen(PATH), taille_nom); //"cutting" the name from its path
            CUT_PATH[taille_nom++] = '\0';

            if( is_in_array(CUT_PATH, mem) == 0){ //checking if the file is not is the array (to not print in more than once)
                if(arg_l == 1){
                    fill_info_array(hd, &mem);//filling FILE_INFO's array if -l argument is given
                }
                strcpy(mem.NAME[ (mem.NUMBER)++ ], CUT_PATH);//copying CUT_PATH(i.e file/rep name to ARRAY)
            }
        }
        //allow to jump to the next hd block
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        lseek(fd,512*nb_bloc_fichier, SEEK_CUR);
    }
    print_ls_to_STROUT(arg_l, mem);
    clear_struct(&mem);
}

int is_in_array(char *string, struct ls_memory mem){ //checking if string is in ls_memory's NAME array
    for(int i = 0; i < mem.NUMBER; i++){
        if(strcmp(string, mem.NAME[i]) == 0) return 1;
    }
    return 0;
}

char is_file_or_repository(char typeflag){
    if(typeflag == '0') return '-';
        return 'd';
}

void fill_info_array(struct posix_header hd, struct ls_memory *mem){ //filling ls_memory's INFO array
    int filesize = 0;
    sscanf(hd.size,"%o",&filesize);
    char c[(strlen(hd.uname)+strlen(hd.gname)+strlen(hd.size)+12) * sizeof(char)];
    sprintf(c, "%c%s %s %s %d", is_file_or_repository(hd.typeflag), octal_to_string(hd.mode), hd.uname, hd.gname, filesize);
    strcpy(mem->INFO[mem->NUMBER], c);
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
    if(mem.NUMBER > 0){ //if there is at least one file/directory to display
        if(arg_l == 0) print_ls(mem);
        else print_ls_l(mem);
    }
}

void clear_struct(struct ls_memory *mem){
    mem-> NUMBER = 0;
    memset(mem->NAME, 0, sizeof(mem->NAME));
    memset(mem->INFO, 0, sizeof(mem->INFO));
}
