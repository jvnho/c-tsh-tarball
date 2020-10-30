#include "bloc.h"
void *my_malloc(size_t size){
    void *result;
    result = NULL;
    while(result == NULL){
        result = malloc(size);
    }
    return result;
}

