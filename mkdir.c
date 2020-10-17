#include <string.h>
#include <stdlib.h>
#include <unistd.h>
struct posix_header
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
  char junk[12];                /* 500 */
};     
char * concatString2(char * path, char *dir){
    
    int length = strlen(path)+strlen(dir)+2;
    char * result = malloc(length);
    strcpy(result, path);
    result[strlen(path)] = '\0';
    strcat(result, dir);
    result[length-2] = '/';
    result[length-1] = '\0';
    return result;
}
struct posix_header create_header(char * name){
    struct posix_header result;
    strcpy(result.name, name);//add the name
    strcpy(result.mode, "000755");
    result.uid[0] = '\0';//how to get the id of the creator
    result.gid[0] = '\0';//how to get the id of the group
    strcpy(result.size, "00000000000 13742577272 013525");
    result.mtime[0] = '\0';//how to get the time of the arrchivement
    result.chksum[0] = '\0';//how to get checksum
    result.typeflag = '5';
    result.linkname[0] = '\0';          
    strcpy(result.magic, "ustar");                
    result.version[0] = '\0';//depends of the user
    result.uname[0] = '\0';//how to get the user name
    result.gname[0] = '\0';//how to get group name
    strcpy(result.devmajor, "000000");
    strcpy(result.devminor, "000000");             
    result.prefix[0] = '\0';             
    result.junk[0]= '\0';  
    return result;
}
void mkdir(char *dir_name, char *fake_path, char *tar_descriptor){
    char *name = concatString2(fake_path, dir_name);
    struct posix_header new_head = create_header(name);
    lseek(atoi(tar_descriptor), -2, SEEK_END);//because at the end of a tar file we have 2 null
    char end = '\0';
    write(atoi(tar_descriptor), &new_head, sizeof(struct posix_header));
    write(atoi(tar_descriptor), &end, sizeof(char));
    write(atoi(tar_descriptor), &end, sizeof(char));
}