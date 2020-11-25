#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tar.h>
#include <unistd.h>
#include <sys/wait.h>

#include "tar.h"
#include "tsh_memory.h"
#include "rmdir.h"
#include "string_traitement.h"

int rmdir_in_tar(int, char*);
int occ_counter_path(int, char*, off_t*, off_t*);

int rmdir_func(tsh_memory *mem, char *dir){
    if(in_a_tar(mem)){ //if the user is in a tar
        rmdir_in_tar(atoi(mem->tar_descriptor), concatString(mem->FAKE_PATH, "arg"));
    } else { //otherwise, we exec the normal rmdir on the current path
        int pid = fork();
        if(pid == 0){ //child processus
            execlp("rmdir", "rmdir", "arg", NULL);
        } else { //parent processus
            int status;
            waitpid(pid, &status, WUNTRACED);
            if(WEXITSTATUS(status) == -1 )
                return -1;
        }
    }
    return 0;
}

int rmdir_in_tar(int fd, char* full_path){
    struct posix_header hd;
    off_t file_offset = 0; //offset will allow to start reading from a certain position
    off_t final_size = 0; //will help us now how much we truncate
    int occurrence = occ_counter_path(fd, full_path, &file_offset, &final_size);
    if(occurrence != 1){
        char s[] = "Directory is not empty or does not exist.\n";
        write(2, s, strlen(s));
        return -1;
    }
    // we found one AND only ONE block so we proced to shift blocks
    lseek(fd,file_offset, SEEK_SET); //starting from the end of the block we want to delete
    while(read(fd, &hd, BLOCKSIZE) > 0){
        lseek(fd, (-BLOCKSIZE*2), SEEK_CUR); //going back to the last block
        write(fd, &hd, BLOCKSIZE); //overwriting the block
        lseek(fd, BLOCKSIZE, SEEK_CUR); //repositionning the offset to the next replacing block
    }
    //procedure to truncate the tar
    final_size = (final_size-1) * 512;
    lseek(fd,0,SEEK_SET);
    ftruncate(fd, final_size);
    return 1;
}

//returns the number of times the path appears in the tarball
//returns to a pointer 'file_offset' the position of the rep and the number of blocks found
int occ_counter_path(int fd, char* full_path, off_t* file_offset, off_t *nb_bloc){
    lseek(fd, 0, SEEK_SET);
    int occurence = 0;
    struct posix_header hd;
    while(read(fd, &hd, 512) > 0){//reading the entire tarball
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + 512 -1) / 512;
        (*nb_bloc) += 1+nb_bloc_fichier;
        if(strncmp(hd.name, full_path, strlen(full_path)) == 0){
            occurence++;
            if(hd.typeflag == '5'){
                *file_offset = lseek(fd,0,SEEK_CUR);//position of the blocks RIGHT NEXT to the one the user wants to delete
            }
        }

        lseek(fd,nb_bloc_fichier*512, SEEK_CUR); //allow to jump to the next header block
    }
    return occurence;
}
