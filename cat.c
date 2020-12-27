#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "tar.h"
#include "tsh_memory.h"
#include "string_traitement.h"

#include "cd.h"


tsh_memory old_memory;


int stop = 0; // stop the cat when giver without argument

void cat_in_tar(int fd, char* PATH){ //test tar
    lseek(fd,0,SEEK_SET);
    struct posix_header hd;
    memset(&hd,0,512);

    while(read(fd,&hd,BLOCKSIZE) > 0){
        int file_s = 0;
        sscanf(hd.size,"%o", &file_s);
        int nb_bloc_fichier = (file_s + 512 -1) / 512;
        if(strcmp(hd.name,PATH)==0){
            char buffer[nb_bloc_fichier*512];
            read(fd, buffer, BLOCKSIZE);
            write(1,buffer,strlen(buffer));
        }

        //next header block
        lseek(fd,nb_bloc_fichier*512,SEEK_SET);

    }
    
    
}
