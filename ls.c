#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "tar.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int is_in_array(char*);
void print_ls_to_STROUT(int);
void free_array_of_string();
void fill_info_array(struct posix_header);
char* octal_to_string(char*);
char is_file_or_repository(char);

char ARRAY[128][255]; //allow to keep the file or repository to display
char FILE_INFO[128][255]; // allow to keep file info (i.e size, uname, gname,...) if -l is given as argument
int NUM_FILE = 0; //keeps a track of the size of ARRAY
char FILE_PATH[255], CUT_PATH[255];//FILE_PATH: path of the file , CUT_PATH = filename cut from the path

int ls(int fd, char* PATH, int arg_l){
    lseek(fd, 0, SEEK_SET);
    struct posix_header hd;
    while(read(fd, &hd, BLOCKSIZE) > 0){ //reading the entire tarball
        strncpy(FILE_PATH, hd.name, strlen(PATH));//splitting the file path to make it matched with the given PATH
        //checking if the current file/repository belongs to the given PATH and if it's not itself (to not print in)
        if(strcmp(FILE_PATH,PATH) == 0 && strcmp(FILE_PATH,hd.name) != 0){

            int i = strlen(PATH), taille_nom = 0;
            while(hd.name[i] != '\0' && hd.name[i] != '/' ){
                i++; taille_nom++;
            }
            strncpy(CUT_PATH, hd.name+strlen(PATH), taille_nom); //"cutting" the filepath
            CUT_PATH[taille_nom++] = '\0';

            if( is_in_array(CUT_PATH) == 0){ //checking if the file is not is the array (to not print in more than once)
                if(arg_l == 1){
                    fill_info_array(hd);//making FILE_INFO's array if -l argument is given
                }
                strcpy(ARRAY[NUM_FILE++], CUT_PATH);//copying CUT_PATH(i.e file/rep name to ARRAY)
            }
        }
        //allow to jump to the next hd block
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, &hd, BLOCKSIZE);
    }
    print_ls_to_STROUT(arg_l);
    return 1;
}

int is_in_array(char *string){ //checking if string is in ARRAY
    for(int i = 0; i < NUM_FILE; i++){
        if(strcmp(string, ARRAY[i]) == 0) return 1;
    }
    return 0;
}

void fill_info_array(struct posix_header hd){
    int filesize = 0;
    sscanf(hd.size,"%o",&filesize);
    char *c = malloc( (strlen(hd.uname)+strlen(hd.gname)+strlen(hd.size)+12) * sizeof(char));
    sprintf(c, "%c%s %s %s %d", is_file_or_repository(hd.typeflag), octal_to_string(hd.mode), hd.uname, hd.gname, filesize);
    strcpy(FILE_INFO[NUM_FILE], c);
}

void print_ls_to_STROUT(int arg_l){
    if(arg_l == 0){
        for(int i = 0; i < NUM_FILE; i++)
            write(1, strcat(ARRAY[i]," "), strlen(ARRAY[i])+2);
        write(1,"\n",1);
    } else { //print with -l
        for(int i = 0; i < NUM_FILE; i++){
            write(1, strcat(FILE_INFO[i]," "), strlen(FILE_INFO[i])+1);
            write(1, strcat(ARRAY[i]," \n"), strlen(ARRAY[i])+2);
        }
    }
}

char* octal_to_string(char *mode){
    char *ret = malloc(sizeof(char)*9);
    ret[0] = '\0';
        for(int i = 0; i < strlen(mode); i++){
        switch(mode[i]){
            case '1': strcat(ret,"r--"); break;
            case '2': strcat(ret,"-w-"); break;
            case '4': strcat(ret,"--x"); break;
            case '3': strcat(ret,"rw-"); break;
            case '5': strcat(ret,"r-x"); break;
            case '6': strcat(ret,"-wx"); break;
            case '7': strcat(ret,"rwx"); break;
            default: break;//if char == 'zero' it does nothing (i.e mode is 00666, 00111,...)
        }
    }
    return ret;
}

char is_file_or_repository(char typeflag){
    if(typeflag == '0')
        return '-';
    else
        return 'd';
}

///////// TEST /////////
 int main(int argc, char * argv[]){
     int fd = open(argv[1], O_RDONLY);
     int ret = 0;
     if(argv[2] != NULL)
        ret = ls(fd, argv[2], 0);
    else
       ret = ls(fd,"", 0);
}
