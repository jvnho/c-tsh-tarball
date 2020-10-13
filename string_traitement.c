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
    char ** result = malloc((dim1 +3)*sizeof(char *));//+2 for the fd_path and the fd_tar + pour le null
    for(int i=0; i<dim1; i++){
        result[i] = malloc((pos[i].fin - pos[i].debut+2) * sizeof(char));//because of '\0'
    }
    return result;
}
char ** to_array_of_string(int *nb, char *args, char *path_fd, char *tar_fd){//to create a char**
    *nb = count_args(args);
    position_mots * position = get_beginEnd_substring(args, *nb);//for the 2 fd, 1 pour  le null
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
    result[*nb] = path_fd;
    result[(*nb)+1] = tar_fd;
    result[(*nb)+2] = NULL;
    *nb = *nb + 2;
    free(position);
    return result;
}
void freeCommand(char **command, int size){
    for(int i=0; i<size; i++){
        free(command[i]);
    }
    free(command);
}
int char_to_int(char a){
    if(a == '0')return 0;
    if(a == '1')return 1;
    if(a == '2')return 2;
    if(a == '3')return 3;
    if(a == '4')return 4;
    if(a == '5')return 5;
    if(a == '6')return 6;
    if(a == '7')return 7;
    if(a == '8')return 8;
    if(a == '9')return 9;
    return -1;
}
int string_to_int(char *chiffre){
    int puissance10 = 1;
    int resultat = 0;
    int tail_chiffre = strlen(chiffre);
    for(int i = (tail_chiffre-1); 0<=i; i--){
        resultat = resultat + char_to_int(chiffre[i]) * puissance10;
        puissance10 *= 10;
    }
    return resultat;
}
int getDigitLength(int chiffre){//the numbre of digit 
    int counter = 0;
    while(chiffre%10!=0){
        counter++;
        chiffre/=10;
    }
    return counter;
}
char int_to_char(int a){
    if(a == 0)return '0';
    if(a == 1)return '1';
    if(a == 2)return '2';
    if(a == 3)return '3';
    if(a == 4)return '4';
    if(a == 5)return '5';
    if(a == 6)return '6';
    if(a == 7)return '7';
    if(a == 8)return '8';
    if(a == 9)return '9';
    return '-';
}
char *int_to_string(int chiffre){
    int size = getDigitLength(chiffre);
    char *result = malloc(size+1);
    int digit = 0;
    for(int i = (size-1); 0<=i; i--){
        digit = chiffre%10;
        result[i] = int_to_char(digit);
        chiffre/=10;
    }
    result[size] = '\0';
    return result;
}
//voir le cas ou dir se termine par un slach
char * concatString(char * path, char *dir){
    
    int length = strlen(path)+strlen(dir)+2;
    char * result = malloc(length);
    strcpy(result, path);
    result[strlen(path)] = '\0';
    strcat(result, dir);
    result[length-2] = '/';
    result[length-1] = '\0';
    return result;
}
