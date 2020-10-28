#include <unistd.h>
#include <sys/types.h> /* A SUPPR INCLUDE DE "OPEN"*/
#include <sys/stat.h>
#include <fcntl.h>

#include "tar.h"
#include "string_traitement.h"

int rm_a_file(int, char*,int *,off_t *);
int rm_a_directory(int, char*,int *,off_t *);
int is_a_end_bloc(struct posix_header*);

int rm_in_tar(int fd, char* full_path, int arg_r){
    int nb_content_bloc = 0;
    off_t file_offset = 0;

    //rm or rm -R
    if(arg_r == 0){
        if(rm_a_file(fd, full_path, &nb_content_bloc, &file_offset) < 0 ) return -1;
    } else if(arg_r == 1){
        if(rm_a_directory(fd, full_path, &nb_content_bloc, &file_offset) < 0 ) return -1;
    } else return -1; //safety

    struct posix_header hd;
    lseek(fd, file_offset + (nb_content_bloc * 512), SEEK_SET);//positionning next to the file user wants to delete
    while(read(fd, &hd, 512) > 0 ){
        lseek(fd, -nb_content_bloc*512 - 512*2, SEEK_CUR); //repositionning back to the block we want to replace
        if(is_a_end_bloc(&hd) != 1){ //if it's not a zero byte block
            write(fd, &hd, 512); //overwriting the block
            lseek(fd, nb_content_bloc*512 + 512, SEEK_CUR); //repositionning the offset to the replacing block
        } else
            //reaching a zero byte block meaning the un-replaced blocks
            //will be replaced by zero byte blocks
            break;
    }

    char zero[512];
    memset(zero,0,512);
    while(read(fd, &hd, BLOCKSIZE) > 0) { //overwriting remaining blocks with zero byte block
        lseek(fd,(-512), SEEK_CUR);
        write(fd, zero, BLOCKSIZE);
    }

    if(arg_r == 0)
        return 1;
    else{
        if(rm_a_directory(fd, full_path, &nb_content_bloc, &file_offset) == -1 )
            return 1;
        else return rm_in_tar(fd,full_path,arg_r);//there is still more directory to delete
    }
}

//function returns 1(and the file offset and number of content blocks) if the file given exists else -1
int rm_a_file(int fd, char* full_path, int *nb_content_bloc, off_t *file_offset){
    lseek(fd, 0, SEEK_SET);

    int filesize = 0;
    struct posix_header hd;

    while(read(fd, &hd, BLOCKSIZE) > 0){
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
        lseek(fd,nb_bloc_fichier*512, SEEK_CUR);
    }
    return -1;//no such file
}

int rm_a_directory(int fd, char * full_path, int *nb_block_to_delete, off_t * debut){
    lseek(fd, 0, SEEK_SET);
    struct posix_header hd;

    while(read(fd, &hd, BLOCKSIZE) > 0){
        int filesize = 0;
        int nb_bloc_fichier = 0;
        if(strncmp(hd.name, full_path, strlen(full_path)) == 0){
            *debut = lseek(fd, 0, SEEK_CUR);
            while(strncmp(hd.name, full_path,strlen(full_path)) == 0){
                sscanf(hd.size, "%o", &filesize);
                nb_bloc_fichier = (filesize + BLOCKSIZE -1) / BLOCKSIZE;
                *nb_block_to_delete += 1 + nb_bloc_fichier;

                lseek(fd, nb_bloc_fichier*512, SEEK_CUR);//jump to the next header
                read(fd,&hd, BLOCKSIZE);
            }
            *nb_block_to_delete -= 1; //not counting the first header (fd starting nex to the first header)
            return 1;
        }
        //jumping to the next file header
        sscanf(hd.size, "%o", &filesize);
        nb_bloc_fichier = (filesize + BLOCKSIZE -1) / BLOCKSIZE;
        lseek(fd, nb_bloc_fichier*512, SEEK_CUR);//jump to the next header
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
// 
// int main(void){
//     int fd = open("f.tar", O_RDWR);
//     rm_in_tar(fd,"pasvide/",1);
// }
