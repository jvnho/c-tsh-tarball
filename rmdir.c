#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tar.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "tar.h"
#include "tsh_memory.h"

int rmdir_in_tar(int, char*);

char FILE_PATH[512];//will allow to save the path and ONLY the path of the file pointed by the posix_header

// int rmdir_func(tsh_memory *mem){
//     if(in_a_tar(mem)){ //if the user is in a tar
//         rmdir_in_tar(atoi(mem->tar_descriptor), mem->FAKE_PATH);
//     } else { //otherwise, we exec the normal rmdir on the current path
//         int pid = fork();
//         if(pid == 0){ //child processus
//             execlp("rmdir", "rmdir", NULL);
//         } else { //parent processus
//             int status;
//             waitpid(pid, &status, WUNTRACED);
//             if(WEXITSTATUS(status) == -1 )
//                 return -1;
//         }
//     }
// }

int occ_counter_path(int fd, char* full_path, off_t* file_offset){//returns the number of times the path appears in the tarball and returns to a pointer 'file_offset' the position of the rep
    lseek(fd, 0, SEEK_SET);
    int occurence = 0;
    struct posix_header hd;
    while(read(fd, &hd, 512) > 0){//reading the entire tarball
        strncpy(FILE_PATH, hd.name, strlen(full_path));
        if(strcmp(FILE_PATH, full_path) == 0){
            if(hd.typeflag == '5'){
                *file_offset = lseek(fd,0,SEEK_CUR);
            }
            occurence++;
        }
        //allow to jump to the next header block
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, &hd, BLOCKSIZE);
    }
    return occurence;
}

int rmdir_in_tar(int fd, char* full_path){
    struct posix_header hd;
    //offset will allow to start reading the tarball from a certain offset and not necessarily the beginning of the tarball
    off_t file_offset = 0;
    if(occ_counter_path(fd, full_path, &file_offset) != 1){
        char s[] = "repository is not empty\n";
        write(1, s, strlen(s));
        return 0;//the repository is not empty or not found then it does nothing and returns 0
    }
    // procedure to shift blocks
    lseek(fd,file_offset, SEEK_SET); //starting from the file the user wants to delete
    while(read(fd, &hd, BLOCKSIZE) > 0){ //to the end of the tar
        lseek(fd, (-BLOCKSIZE*2), SEEK_CUR); //going back to the last block
        write(fd, &hd, BLOCKSIZE); //writes what it has it the posix_header struct (and move the offset to next block)
        lseek(fd, BLOCKSIZE, SEEK_CUR); //repositionning the offset to the one more block
    }
    return 1;
}

char * concate_path_rep(char *PATH, char *rep_name){//concating tar path with the empty repository name user wants to delete
    char *ret = malloc( strlen(PATH) + strlen(rep_name) + 1);
    sprintf(ret, "%s%s%s", PATH, rep_name, "\0");
    return ret;
}

///////// TEST /////////
// int main(int argc, char **argv){
//     int fd = open(argv[1], O_RDWR);
//     rmdir_in_tar(fd, concate_path_rep(argv[2],argv[3]));
// }