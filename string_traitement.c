#include <string.h>
#include <stdio.h>
int count_args(char *args){
    int counter = 0;
    int get_into_char = 0;
    int length = strlen(args);
    for(int i=0; i<length; i++){
        if(args[i]!=' ' && get_into_char==0){//we read the first char or a word
            counter++;
            get_into_char = 1;
        }
        if(args[i]==' '){
            get_into_char = 0;
        }
    }
    return counter;
}
/*char ** getCommand(int *nb, char *args){

}*/
int main(void){
    printf("tail = %d\n", count_args(" cd   System/   cours"));
    return 0;
}
