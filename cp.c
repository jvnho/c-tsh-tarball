#include <stdio.h>
#include <string.h>
#include "bloc.h"
int main(void){
    //teste de la fonction my_malloc
    content_bloc teste[12];
    strcpy(teste[0].name, "nom");
    printf("n = %s\n", teste[0].name);
    return 0;
}