#include <string.h>
#include <stdio.h>
int count_args(char *args){
    int counter = 0;
    int if_space = 0;
    int get_into_char = 0;
    int length = strlen(args);
    for(int i=0; i<length; i++){
        if(args[i]!=' ' && get_into_char==0){//we read the first char or a word
            counter++;
        }
    }
    return 0;
}
/*char ** getCommand(int *nb, char *args){

}*/
int main(void){
    printf("teste");
    return 0;
}
