#include <unistd.h>
/*
    source = dossier1/          target = cheminDansTar/
    1) cherche -> FakePath/dossier1/
    2) a) trouver -> FakePath/dossier1/truc
        b) creer -> cheminDansTar/dossier1/truc

    structur positionHeader
        position lseek
        int nb_block
*/
//to write a content bloc from a char ** into the .tar
int writeContentBloc_in_tar(char **source, int fd_tar){
    return 0;
}
int cp_in_tar(char *source, char *target, char *path, int tar_fd){

    return 0;
}
int main(void){

    return 0;
}