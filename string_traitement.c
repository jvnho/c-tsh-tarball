#include <string.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct position_mots{
    int debut;
    int fin;
}position_mots;
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
position_mots *get_beginEnd_substring(char *args, int nb_substring){
    position_mots *result = malloc(nb_substring * sizeof(position_mots));
    int get_into_char = 0;
    int get_into_space = 0;
    int length = strlen(args);
    int result_index = 0;
    for(int i=0; i<length; i++){
        if(args[i]!=' ' && get_into_char==0){//we read the first char or a word
            result[result_index].debut = i;
            get_into_char = 1;
            get_into_space = 0;
        }
        if(args[i]==' '){
            if(get_into_space == 0 && i!=0){//first time we get into a space, that mean the previsous index is the end of a word
                result[result_index].fin = i-1;
                result_index++;
            }
            get_into_space = 1;
            get_into_char = 0;
        }
    }
    if(result[nb_substring-1].fin == 0)result[nb_substring-1].fin = strlen(args)-1;
    return result;
}
/*char ** getCommand(int *nb, char *args){

}*/
int main(void){
    int nb_args = count_args(" cd   System/   cours");
    position_mots *teste = get_beginEnd_substring(" cd   System/   cours", nb_args);
    for(int i = 0; i<nb_args; i++){
        printf("debut = %d    fin = %d\n", teste[i].debut, teste[i].fin);
    }
    return 0;
}
