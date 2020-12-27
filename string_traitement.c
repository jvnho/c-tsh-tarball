#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "tsh_memory.h"
#define BUFSIZE 512
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
char *int_to_string(int chiffre, char *result){
    memset(result, 0, 512);
    int size = getDigitLength(chiffre);
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
char * concatDirToPath(char * path, char *dir){

    int length = strlen(path)+strlen(dir)+2;
    char * result = malloc(length);
    strcpy(result, path);
    result[strlen(path)] = '\0';
    strcat(result, dir);
    if(dir[strlen(dir)-1] == '/'){
        result[length - 2] = '\0';
    }else{
        result[length-2] = '/';
        result[length-1] = '\0';
    }
    return result;
}
char * simpleConcat(char *path, char *dir){
    int length = strlen(path) + strlen(dir) + 1;
    char *result = malloc(length);
    strcpy(result, path);
    result[strlen(path)] = '\0';
    strcat(result, dir);
    result[length - 1] = '\0';
    return result;
}

char *concate_string(char *s1, char *s2){
    char *ret = malloc((strlen(s1)+strlen(s2)+1)*sizeof(char));
    sprintf(ret,"%s%s%c", s1, s2, '\0');
    return ret;
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
    memset(result, 0, 512);
    int first_slach_beforTar = get_index_first_slach(initial_string);
    if(first_slach_beforTar == 0) return;//there is not a pre tar
    else if(first_slach_beforTar == -1){//the is not a .tar, so everithing is a pre tar
        strcpy(result, initial_string);
    }else{
        //copy destionation source(from where) size(how many char)
        memcpy(result, initial_string, first_slach_beforTar + 1);// +1 because index start from 0
        result[first_slach_beforTar + 1] = '\0';
    }
}
//return the substring matched to the tar name
void getTarName(char *initial_string, char *result){
    memset(result, 0, 512);
    int first_slach_beforTar = get_index_first_slach(initial_string);
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
//
void getPostTar(char *initial_string, char *result){
    memset(result, 0, 512);
    char *substring = memmem(initial_string, strlen(initial_string), ".tar", strlen(".tar"));
    int len_initial = strlen(initial_string);
    //there is not a .tar so there is not an after .tar
    if(substring == NULL)return;
    int begin_index = substring - initial_string + strlen(".tar/");//from where we take the substring
    if(begin_index > len_initial) return;//the .tar is at the end -> also don't have a postTar
    memcpy(result, initial_string + begin_index, len_initial - begin_index + 1);
    result[len_initial - begin_index + 1] = '\0';
}

//returns the position of the (second) slash from the end, if not found it returns -1
int get_prev_directory(char *path){
    int index = strlen(path)-2; //PATH[strlen(PATH)-2] already equals to a slash
    while(index >= 0){
        if(path[index] == '/')
            return index;
        index--;
    }
    return -1;
}

void octal_to_string(char *mode, char *result){
    memset(result, 0, sizeof(result));
    for(int i = 0; i < strlen(mode); i++)
    {
        switch(mode[i]){
            case '1': strcat(result,"r--"); break;
            case '2': strcat(result,"-w-"); break;
            case '4': strcat(result,"--x"); break;
            case '3': strcat(result,"rw-"); break;
            case '5': strcat(result,"r-x"); break;
            case '6': strcat(result,"-wx"); break;
            case '7': strcat(result,"rwx"); break;
            default: break;//if char == 'zero' it does nothing (i.e mode is 00666, 00111,...)
        }
    }
}
int firstSlach(char *dir){
    int len = strlen(dir);
    for(int i = 0; i<len; i++){
        if(dir[i]=='/')return i;
    }
    return -1;
}
//fill result of the first dir/ in source into the result
int getFirstDir(char *source, char *result){
    memset(result, 0, sizeof(char)*512);
    int index_first_slach = firstSlach(source);
    memcpy(result, source, index_first_slach + 1);
    return index_first_slach;
}
char **addNullEnd(char **initial, int size){
    char **result;
    assert(result = malloc((size + 1)*sizeof(char *)));
    for(int i = 0; i< size; i++){
        assert(result[i] = malloc(strlen(initial[i])*sizeof(char)));
        strcpy(result[i], initial[i]);
    }
    result[size] = NULL;
    return result;
}
//remove "./", "./" ,"/./" from str
void remove_simple_dot_from_dir(char *str){
    //beginning of the string
    while(str[0] == '.' && (str[1] != '.' && str[1] == '/')){ // if "././././dos1" entered
        strcpy(str,str+2);
    }

    //end of the string
    while(1){
        int str_length = strlen(str);
        if(str[str_length-1] == '/' && str[str_length-2] == '.' && str[str_length-3] != '.'){ // cd doss1/./
            strncpy(str, str, str_length-2);
            str[str_length-2] = '\0';
        } else if(str[str_length-1] == '.' && str[str_length-2] != '.'){ // cd doss1/.
            str[str_length-1] = '\0';
        } else break;
    }
    //anywhere in middle of the string
    char *tmp;
    while( (tmp = strstr(str,"/./")) != NULL){ //while there are /./ found in the string
        int length = tmp - str ; //number of char before the "/."
        char *buf = malloc(sizeof(char)* (strlen(str)-2));
        strncpy(buf, str, length);
        strcat(buf,tmp+2);
        strcpy(str,buf);
    }
}
int getIndexFirstSlachBehind(char *source){
    int len = strlen(source);
    for(int i = len -2; 0<=i; i--){
        if(source[i]=='/')return i;
    }
    return -1;
}
//if source = aa/bb/cc/dossier => result = aa/bb/cc
void getLocation(char *source, char *result){
    memset(result, 0, 512);
    int indexSlach = getIndexFirstSlachBehind(source);
    if(indexSlach == -1)return;
    //copy destionation source(from where) size(how many char)
    memcpy(result, source, indexSlach + 1);// +1 because index start from 0
}

int is_unix_directory(char *str){
    char location[512];
    getLocation(str,location);
    char *str2 = str + strlen(location);
    return ( strcmp(str2,".") == 0 || strcmp(str2,"./") == 0 || strcmp(str2,"..") == 0 || strcmp(str2,"../") == 0);
}

int is_extension_tar(char *str){
    return (str[strlen(str)-1] == 'r' && str[strlen(str)-2] == 'a' && str[strlen(str)-3] == 't' && str[strlen(str)-4] == '.');
}
//split the command in to on the "|" in order to execute it separly
int spilitPipe(tsh_memory *source, tsh_memory *memory1, tsh_memory *memory2){
    copyMemory(source, memory1);//copy the context of execution
    copyMemory(source, memory2);
    memset(memory1->comand, 0, 512);//clear the command
    memset(memory2->comand, 0, 512);
    char *tok;
    if((tok = strtok(source->comand, "|")) == NULL)return -1;
    strcpy(memory1->comand, tok);//initialize the command of mem1 to the command befor '|'
    if(memory1->comand[strlen(memory1->comand)-1] == ' ')memory1->comand[strlen(memory1->comand)-1] = '\0';//remove the space  at the end
    if((tok = strtok(NULL, "|")) == NULL)return -1;//initialize the second part of the command
    if(tok[0] == ' ')strcpy(memory2->comand, tok+1);
    else strcpy(memory2->comand, tok);
    return 0;
}

void get_tar_path(tsh_memory *memory, char *container, char *abs_path){
    printf("abs_path %s\n", abs_path);
    memset(container,0, 512);
    char *sub = strstr(abs_path, ".tar");
    int starting = sub - abs_path;
    strcpy(container, abs_path+starting+5);
}
//add slach betwen
void concatenationPath(char *first, char *second, char *result){//sans malloc
    memset(result, 0, 512);
    strcpy(result, first);
    result[strlen(first)] = '/';
    strcat(result, second);
}
//witout slach
void concatenation(char *first, char *second, char *result){
    memset(result, 0, 512);
    strcpy(result, first);
    strcat(result, second);
}

