#include <string.h>
#include <unistd.h>
#include "tar.h" 
#include "string_traitement.h"
#include "tsh_memory.h"
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
void mkdir(char *dir_name, tsh_memory *memory){
    char *name = concatString(memory->FAKE_PATH, dir_name);
    struct posix_header new_head = create_header(name);
    lseek(string_to_int(memory->tar_descriptor), -2, SEEK_END);//because at the end of a tar file we have 2 null
    char end = '\0';
    write(string_to_int(memory->tar_descriptor), &new_head, sizeof(struct posix_header));
    write(string_to_int(memory->tar_descriptor), &end, sizeof(char));
    write(string_to_int(memory->tar_descriptor), &end, sizeof(char));
}