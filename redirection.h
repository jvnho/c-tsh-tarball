#ifndef REDIRECTION_H
#define REDIRECTION_H

#define NB_ENTRY 512
#define MAX_SIZE 1024

typedef struct redirection_array{
    char OUTPUT_NAME[NB_ENTRY][MAX_SIZE]; //name of the redirection target
    int APPEND[NB_ENTRY]; //1 or 0
    int NUMBER; //number of redirection
} redirection_array;


#endif