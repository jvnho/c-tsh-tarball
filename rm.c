#include <unistd.h>

#include "tar.h"
#include "string_traitement.h"
#include "tsh_memory.h"

int rm_aux(int, char*,int*,off_t*,int);
int is_a_end_bloc(struct posix_header*);
int number_of_block(int fd);

int rm(tsh_memory *mem){
    return 0;
}

//full_path is the concatenation of PATH and the "target" user wants to delete (rm)
int rm_in_tar(int fd, char* full_path, int arg_r, int first_call){
    int nb_content_bloc = 0;
    off_t file_offset = 0, tar_size = 0;
    if(rm_aux(fd, full_path, &nb_content_bloc,&file_offset, arg_r) < 0 ){
        if(first_call == 1){
            char *msg;
            if(arg_r == 0) msg = "File not found or target is a directory.\n";
            else msg = "Directory not found.\n";
            write(2,msg,strlen(msg));
        }
        return -1;
    }
    struct posix_header hd;
    lseek(fd, file_offset + (nb_content_bloc * 512), SEEK_SET);//positionning fd after all the content blocks of "target"
    while(read(fd, &hd, 512) > 0 ){
        lseek(fd, -nb_content_bloc*512 - 512*2, SEEK_CUR); //repositionning fd back to the block we want to replace
        if(is_a_end_bloc(&hd) != 1){ //if what we read is not a zero byte block
            write(fd, &hd, 512); //overwriting the block by what we read previously
            lseek(fd, nb_content_bloc*512 + 512, SEEK_CUR); //repositionning the offset to the next replacing block
        } else break;
            //reaching a zero byte block meaning the un-replaced blocks will be replaced by zero byte blocks
    }
    //overwriting remaining blocks with zero byte block
    //but if we've already reached the end of the tar earlier nothing will be done
    char zero[512];
    memset(zero,0,512);
    while(read(fd, &hd, BLOCKSIZE) > 0) {
        lseek(fd,(-512), SEEK_CUR);
        write(fd, zero, BLOCKSIZE);
    }
    //procedure to truncate the tar
    off_t final_size = (number_of_block(fd) - nb_content_bloc+1)*512;
    ftruncate(fd, final_size);
    if(arg_r == 0) return 1;
    else{
        //making sure there is no other blocks to delete
        while(rm_in_tar(fd,full_path,arg_r,0) != -1);
        return 1;
     }
}

//function that counts the number of content block to delete
//returns -1 if such file/directory is not found
int rm_aux(int fd, char* full_path, int *nb_block_to_delete, off_t *debut, int arg_r){
    lseek(fd, 0, SEEK_SET);

    int filesize = 0;
    struct posix_header hd;

    while(read(fd, &hd, BLOCKSIZE) > 0){
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + BLOCKSIZE -1) / BLOCKSIZE;
        if(strncmp(hd.name, full_path, strlen(full_path)) == 0){
            *debut = lseek(fd, 0, SEEK_CUR);
            if(arg_r == 0){
                if(hd.typeflag != '5'){ //if it's not a directory
                    *nb_block_to_delete = nb_bloc_fichier;
                    return 1;
                }
            } else { //argument -r was given as option
                while(strncmp(hd.name, full_path,strlen(full_path)) == 0){
                    sscanf(hd.size, "%o", &filesize);
                    nb_bloc_fichier = (filesize + BLOCKSIZE -1) / BLOCKSIZE;
                    *nb_block_to_delete += 1 + nb_bloc_fichier;

                    lseek(fd, nb_bloc_fichier*512, SEEK_CUR);//jump to the next header
                    read(fd,&hd, BLOCKSIZE);
                }
                *nb_block_to_delete -= 1; //not counting the first header (fd starting next to the first header)
                return 1;
            }
        }
        lseek(fd, nb_bloc_fichier*512, SEEK_CUR); //jumping to the next file header
    }
    return -1;//no such file
}

//create a 512 bytes block of zero byte, in order to compare with header
int is_a_end_bloc(struct posix_header *header){
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    if(memcmp(header, end_bloc, 512) == 0) return 1; //identical
    return 0;
}

//return the number total of block in a tar (i.e its size)
int number_of_block(int fd){
    char buffer[512];
    int ret = 0;
    lseek(fd, 0, SEEK_SET);
    while(read(fd, buffer, 512) > 0)
        ret++;
    return ret;
}
