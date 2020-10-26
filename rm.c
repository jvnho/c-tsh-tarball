#include <unistd.h>
#include <sys/types.h> /* A SUPPR INCLUDE DE "OPEN"*/
#include <sys/stat.h>
#include <fcntl.h>

#include "tar.h"

int get_file_offset(int fd, char* full_path, int *nb_content_bloc, off_t *file_offset){ //returns 1 if the file given exists else -1
    lseek(fd, 0, SEEK_SET);
    struct posix_header hd;
    while(read(fd, &hd, BLOCKSIZE) > 0){
        int filesize = 0;
        sscanf(hd.size, "%o", &filesize);
        int nb_bloc_fichier = (filesize + BLOCKSIZE -1) / BLOCKSIZE;
        if(hd.typeflag != '5'){ //if its not a directory
            if(strcmp(full_path, hd.name) == 0){
                *nb_content_bloc = nb_bloc_fichier;
                *file_offset = lseek(fd, 0, SEEK_CUR);
                return 1;
            }
        }
        //jumping to the next file header
        for(int i = 0; i < nb_bloc_fichier; i++) read(fd, &hd, BLOCKSIZE);
    }
    return -1;//no such file
}

int is_a_end_bloc(struct posix_header *header){
    //create a string that has all the bloc zero byte, in order to compare with header
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    //then compare
    if(memcmp(header, end_bloc, 512) == 0)return 1;//identical
    return 0;
}

int rm_in_tar(int fd, char* full_path){
    int nb_content_bloc = 0;
    off_t file_offset = 0;
    if(get_file_offset(fd, full_path, &nb_content_bloc, &file_offset) < 0 )
        return -1;
    struct posix_header hd;

    lseek(fd, file_offset + (nb_content_bloc * 512), SEEK_SET);//positionning next to the file user wants to delete
    while(read(fd, &hd, 512) > 0 ){
    lseek(fd, -nb_content_bloc*512 - 512*2, SEEK_CUR); //repositionning back to the block we want to replace
        if(is_a_end_bloc(&hd) != 1){ //if it's not a zero byte block
            write(fd, &hd, 512); //writes what there is it the posix_header struct (and move the offset to next block)
            lseek(fd, nb_content_bloc*512 + 512, SEEK_CUR); //repositionning the offset to the replacing block
        } else {
            //reaching a zero byte block meaning the un-replaced blocks
            //will be replaced by zero byte blocks
            break;
        }
    }

    while(read(fd, &hd, BLOCKSIZE) > 0) { //reading the rest of the tarball while replacing blocks by zero byte block
        char zero[512];
        memset(zero,0,512);
        write(fd, zero, BLOCKSIZE);
    }
    return 1;
}

// int main(void){
//     int fd = open("f.tar", O_RDWR);
//     rm_in_tar(fd,"plusvide/zozo");
// }
