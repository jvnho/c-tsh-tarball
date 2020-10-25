#include <unistd.h>
#include <sys/types.h> /* A SUPPR INCLUDE DE "OPEN"*/
#include <sys/stat.h>
#include <fcntl.h>

#include "tar.h"

int get_file_offset(int fd, char* full_path, int *nb_content_bloc, off_t *file_offset){ //returns 1 if the file given exists else -1
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

int rm_in_tar(int fd, char* full_path){
    int nb_content_bloc = 0;
    off_t file_offset = 0;
    if(get_file_offset(fd, full_path, &nb_content_bloc, &file_offset) < 0 ) return -1;

    struct posix_header hd;
    int read_size = 0;

    lseek(fd, file_offset + (nb_content_bloc * BLOCKSIZE), SEEK_SET);
    while( (read_size = read(fd, &hd, BLOCKSIZE)) > 0){ //hd != block nul

        lseek(fd, -(nb_content_bloc * BLOCKSIZE) - BLOCKSIZE*2, SEEK_CUR);

        if(read_size > 0){
            write(fd, &hd, BLOCKSIZE);
        } else { //reached zero bytes block (i.e the end of the file) but we didn't finish to replace all the blocks
            char zero[512];
            memset(zero,0,512);
            write(fd, zero, BLOCKSIZE);
        }

        lseek(fd, (nb_content_bloc * BLOCKSIZE) + BLOCKSIZE, SEEK_CUR);
    }
    return 1;
}

// int main(void){
//     int fd = open("f.tar", O_RDWR);
//     rm_in_tar(fd,"plusvide/zozo");
// }
