#include "ls.h"
#include "cd.h"
#include "tar.h"
#include "tsh_memory.h"
#include "exec_funcs.h"
#include "string_traitement.h"

tsh_memory old_memory; //will be use to save/restore a memory
char **array_execvp;

int is_in_array(char *string, struct ls_memory mem){ //checking if string is in ls_memory's NAME array
    for(int i = 0; i < mem.NUMBER; i++){
        if(strcmp(string, mem.NAME[i]) == 0) return 1;
    }
    return 0;
}

char file_type(char typeflag){
    if(typeflag == '2') return 'l';
    if(typeflag == '5') return 'd';    
    if(typeflag == '6') return 'p';
        return '-';
}

//return the size of a directory in a tarfile
int size_of_directory(int fd, off_t initial, char *path){
    struct posix_header hd;
    lseek(fd, 0, SEEK_CUR); 
    int size = 0;
    while(read(fd, &hd, 512) > 0)
    {
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        if(strncmp(hd.name, path, strlen(path)) == 0)
        {
            if(hd.typeflag != '5') size += filesize;
        }
        lseek(fd,512*nb_bloc_fichier, SEEK_CUR);
    }
    lseek(fd, initial, SEEK_SET);
    return size;
}

//inspiré de https://koor.fr/C/ctime/struct_tm.wp
void epoch_time_converter(char *buffer, char *result){
    memset(result, 0, 80);
    time_t cur_time = atoi(buffer);
    struct tm  ts;
    ts = *localtime(&cur_time);
    strftime(result, sizeof(result), "%b. %d %H:%M", &ts);
}

void fill_info_array(int fd, off_t initial,struct posix_header hd, struct ls_memory *mem){ //filling ls_memory's INFO array
    int filesize = 0;
    if(hd.typeflag == '5') //directory
    { 
        filesize = size_of_directory(fd, initial,hd.name);
    } else {
        sscanf(hd.size,"%o",&filesize);
    }
    char info[512], mode[512], time[80];
    epoch_time_converter(hd.mtime, time);
    octal_to_string(hd.mode, mode);
    sprintf(info, "%c%s %s %s %d %s", file_type(hd.typeflag), mode, hd.uname, hd.gname, filesize, time);
    strcpy(mem->INFO[mem->NUMBER], info);
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

//looking for a single file in the tar given a path and print it
int ls_in_tar_file(int fd, char* full_path, int arg_l){
    struct posix_header hd;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &hd, 512) > 0)
    {
        if(strcmp(hd.name, full_path) == 0 && hd.typeflag != 5)
        {
            int i = strlen(full_path);
            while(hd.name[i-1] != '/' && i > 0) i--;
            print_filename_stdout(hd.name+i);
            return 1;
        }
    }
    return -1; //file not found
}

//looking for a directory: store in an array all file or directory belonging to to directory given
int ls_in_tar_directory(int fd, char* full_path, int arg_l){
    struct posix_header hd;
    struct ls_memory mem;
    clear_struct(&mem);
    int len_path = strlen(full_path), fic_found = 0;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, &hd, 512) > 0)
    {
        if(strncmp(hd.name, full_path, len_path)==0 && strcmp(hd.name, full_path)!=0) //notice strncmp & strcmp
        {
            char CUT_PATH[255];//CUT_PATH = file or directory name cut from its path
            int starting = len_path, len_name = 0;
            while(hd.name[starting] != '\0' && hd.name[starting] != '/' ){
                starting++; len_name++;
            }
            strncpy(CUT_PATH, hd.name+len_path, len_name); //"cutting" the name from its path
            CUT_PATH[len_name++] = '\0';
            if(is_in_array(CUT_PATH, mem) == 0)//avoiding dupplicates
            { 
                if(arg_l == 1) fill_info_array(fd, lseek(fd, 0, SEEK_CUR),hd, &mem);//filling FILE_INFO's array if -l argument is given
                strcpy(mem.NAME[mem.NUMBER], CUT_PATH);
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
    if(mem.NUMBER > 0) print_ls_to_STROUT(arg_l, mem); //there is at least one thing to display
    return fic_found;
}

//execution of a ls on one argument
void do_ls(tsh_memory *memory, char *dir, char option[50][50], int nb_option, int l_opt){
    copyMemory(memory,&old_memory); //saving current state of the tsh_memory

    if(dir[strlen(dir)-1] == '/'  || is_unix_directory(dir) == 1) //we are sure the user wants to ls a directory (@string_traitement.c)
    { 
        if(cd(dir, memory) > -1) //cd-ing to the directory location (if it exists)
        { 
            if(in_a_tar(memory) == 1) {
                ls_in_tar_directory(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt);
            } else {
                array_execvp = execvp_array("ls", NULL,option,nb_option); //details @functions.h
                exec_cmd("ls", array_execvp);
            }
        }
    } 
    else // at this point, we don't know if the user wants to ls a directory or a file
    { 
        char location[512];
        char *dirToVisit = dir;
        getLocation(dir,location); //@string_traitement.c for details
        if(strlen(location) == 0 && is_extension_tar(dirToVisit) == 1) //in this case, user wants to ls a .tar file so we cd in it and run ls
        { 
            if(cd(dirToVisit, memory) == -1){
                restoreLastState(old_memory, memory);
                return;
            }
            ls_in_tar_directory(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt);
            restoreLastState(old_memory, memory);
            return;
        }
        if(strlen(location) > 0)
        {
            if(cd(location, memory) == -1){
                restoreLastState(old_memory, memory);
                return; //path doesn't exist
            }
            dirToVisit += strlen(location);
        }
        if(in_a_tar(memory) == 1)
        {
            char file_path[512];
            concatenation(memory->FAKE_PATH, dirToVisit,file_path);
            if(ls_in_tar_file(atoi(memory->tar_descriptor), file_path, l_opt) == -1) //trying to find in the tar if the file "dirToVisit" exists
            { 
                if(cd(dirToVisit, memory) != -1){ //trying now to find in the tar if the directory "dirToVisit" exists
                    ls_in_tar_directory(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt); //at this point we are sure we are still in a tar
                }
            }
        } 
        else {
            array_execvp = execvp_array("ls", dirToVisit,option,nb_option);
            exec_cmd("ls", array_execvp);  
        }
    }
    if(array_execvp != NULL)  free(array_execvp);
    restoreLastState(old_memory, memory);
}

int ls(tsh_memory *memory, char args[50][50], int nb_arg, char option[50][50],int nb_option){
    int l_opt = option_present("-l", option, nb_option);
    if(nb_arg == 0)
    {
        if(in_a_tar(memory) == 1) {
            ls_in_tar_directory(atoi(memory->tar_descriptor), memory->FAKE_PATH, l_opt);
        } else { 
            array_execvp = execvp_array("ls", NULL,option,nb_option);
            exec_cmd("ls", array_execvp);
            free(array_execvp);
        }
    } 
    else 
    {
        for(int i = 0; i < nb_arg; i++)
        {
            do_ls(memory, args[i], option, nb_option, l_opt);
        }
    }
    return 1;
}
