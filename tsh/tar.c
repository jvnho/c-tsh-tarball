#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "tar.h"
/* ... */


/* Compute and write the checksum of a header, by adding all
   (unsigned) bytes in it (while hd->chksum is initially all ' ').
   Then hd->chksum is set to contain the octal encoding of this
   sum (on 6 bytes), followed by '\0' and ' '.
*/

void set_checksum(struct posix_header *hd) {
  memset(hd->chksum,' ',8);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  sprintf(hd->chksum,"%06o",sum);
}

/* Check that the checksum of a header is correct */

int check_checksum(struct posix_header *hd) {
  unsigned int checksum;
  sscanf(hd->chksum,"%o ", &checksum);
  unsigned int sum = 0;
  char *p = (char *)hd;
  for (int i=0; i < BLOCKSIZE; i++) { sum += p[i]; }
  for (int i=0;i<8;i++) { sum += ' ' - hd->chksum[i]; }
  return (checksum == sum);
}

//check if the header corepond to an end of bloc
int end_bloc(struct posix_header *header){
    //create a string that has all the bloc zero byte, in order to compare with header
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    //then compare
    if(memcmp(header, end_bloc, 512) == 0)return 1;//identical
    return 0;
}
//like a huge lseek that move to the first zero bloc
void put_at_the_first_null(int descriptor){
    lseek(descriptor, 0, SEEK_SET);
    struct posix_header *header = malloc(512);
    int nb_bloc_file = 0;
    int j = 1;
    while(read(descriptor, header, 512)>0){
        if(end_bloc(header)){//the moment we check the first bloc, we return to the back because we have read it
            lseek(descriptor, -512, SEEK_CUR);
            return;
        }
        int tmp = 0;
        sscanf(header->size, "%o", &tmp);
        nb_bloc_file = (tmp + 512 -1) / 512;
        for(int i=0; i<nb_bloc_file; i++){
            read(descriptor, header, 512);
        }
        j++;
    }
}
//create the exact same header with a different name
struct posix_header copyHeader(struct posix_header initial, char *name){

    struct posix_header result;
    strcpy(result.name, name);//add the name
    strcpy(result.mode, initial.mode);
    strcpy(result.uid, initial.uid);
    strcpy(result.gid, initial.gid);
    int size;
    sscanf(initial.size, "%o", &size);
    sprintf(result.size, "%011o", size);
    sprintf(result.mtime, "%011lo", time(NULL));
    

    result.typeflag = initial.typeflag;
    strcpy(result.linkname, initial.linkname);

    strcpy(result.magic, initial.magic);
    result.version[0] = initial.version[0];
    result.version[1] = initial.version[1];

    strcpy(result.uname, initial.uname);
    strcpy(result.gname, initial.gname);

    strcpy(result.devmajor, initial.devmajor);
    strcpy(result.devminor, initial.devminor);
    strcpy(result.prefix, initial.prefix);
    strcpy(result.junk, initial.junk);
    set_checksum(&result);
    
    return result;
}
struct posix_header create_header(char * name, int dir, int size){

    struct posix_header result;
    memset(&result, 0, 512);
    strcpy(result.name, name);//add the name
    if(dir)sprintf(result.mode, "000755 ");
    else sprintf(result.mode, "000644 ");
        
    sprintf(result.uid, "000765 ");
    sprintf(result.gid, "000024 ");

    sprintf(result.size, "%011o", size);
    //sprintf(result.mtime, "%ld", time(NULL));//au pire metre zero
    sprintf(result.mtime, "%d", 0);//au pire metre zero

    if(dir){result.typeflag = '5';}else result.typeflag = '0';
    result.linkname[0] = '\0';

    strcpy(result.magic, "ustar");
    result.version[0]='0';
    result.version[1]= '0';

    strcpy(result.uname, "sarobidy");//getlogin()
    strcpy(result.gname, "staff");//staff

    strcpy(result.devmajor, "000000 ");
    strcpy(result.devminor, "000000 ");
    result.prefix[0] = '\0';
    result.junk[0]= '\0';
    set_checksum(&result);
    return result;
}

//write a zero bloc at the end
int writeZero(int tar_descriptor){
    lseek(tar_descriptor, 0, SEEK_END);
    char end_bloc[512];
    memset(end_bloc, 0, 512);
    if(write(tar_descriptor, end_bloc, 512)==-1){
        perror("");
        return -1;
    }
    return 0;
}

int dir_exist(int descriptor, char * directory){
    lseek(descriptor, 0, SEEK_SET);
    struct posix_header header;
    int nb_bloc_file = 0;
    while(read(descriptor, &header, 512)>0){//parcour de tete en tete jusqu' a la fin
        
        if(strcmp(header.name, directory)==0){
            return 1;
        }
        int tmp = 0;
        sscanf(header.size, "%o", &tmp);
        nb_bloc_file = (tmp + 512 -1) / 512;
        for(int i=0; i<nb_bloc_file; i++){
            read(descriptor, &header, 512);
        }
    }
    return 0;
}
