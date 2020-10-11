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
char ** allocate_2Dmemory(int dim1, position_mots *pos){
    char ** result = malloc(dim1*sizeof(char *));
    for(int i=0; i<dim1; i++){
        result[i] = malloc((pos[i].fin - pos[i].debut+2) * sizeof(char));//because of '\0'
    }
    return result;
}
char ** getCommand(int *nb, char *args){
    *nb = count_args(args);
    position_mots * position = get_beginEnd_substring(args, *nb);
    char **result = allocate_2Dmemory(*nb, position);
    int index_char = 0;
    for(int i=0; i<*nb; i++){
        for(int j = position[i].debut; j<=position[i].fin; j++){
            result[i][index_char] = args[j];
            index_char++;
        }
        result[i][index_char] = '\0';
        index_char = 0;
    }
    return result;
}
