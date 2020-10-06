#include <stdio.h>
int main(int nb, char **args){
    if(nb<2){
        printf("missing the .tar file to execute this programe\n");
        return -1;
    }
    return 0;
}