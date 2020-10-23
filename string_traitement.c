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
int get_index_first_slach(char *initial_string){
    char * substring = memmem(initial_string, strlen(initial_string), ".tar", strlen(".tar"));
    if(substring == NULL)return -1;//there is not a .tar -> so there is not first slach befor .tar
    int index_point = substring - initial_string;
    for(int i = index_point; 0<=i; i--){
        if(initial_string[i] == '/')return i;
    }
    return 0;
}
//return the substring befor the directory.tar
void getPreTar(char *initial_string, char *result){
    int first_slach_beforTar = get_index_first_slach(initial_string);
    result[0] = '\0';
    if(first_slach_beforTar == 0) return;//there is not a pre tar
    else if(first_slach_beforTar == -1){//the is not a .tar, so everithing is a pre tar
        strcpy(result, initial_string);
    }else{
        //copy destionation source(from where) size(how many char)
        memcpy(result, initial_string, first_slach_beforTar + 1);// +1 because index start from 0
        result[first_slach_beforTar + 1] = '\0';
    }
}
void getTarName(char *initial_string, char *result){
    int first_slach_beforTar = get_index_first_slach(initial_string);
    result[0] = '\0';
    if(first_slach_beforTar == -1)return;//there is nothing to complete
    //cherching the ending index .tar<-
    char *substring = memmem(initial_string, strlen(initial_string), ".tar", strlen(".tar"));
    int ending_index = substring - initial_string + strlen("tar");
    if(first_slach_beforTar == 0){//copy form 0 to ending index
        memcpy(result, initial_string, ending_index + 1);
        result[ending_index + 1] = '\0';
    }else{//copy form first_slach + 1 to ending index
        memcpy(result, initial_string + first_slach_beforTar + 1, ending_index - first_slach_beforTar);
        result[ending_index - first_slach_beforTar] = '\0';
    }
    
}
