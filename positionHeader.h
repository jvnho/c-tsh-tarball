#ifndef POSITIONHEADER_H
#define POSITIONHEADER_H
#include <stdio.h>
typedef struct pos_header{
    off_t positon;
    int nb_block;
}pos_header;
//Pile
typedef struct stackPosition{
    pos_header pos;
    pos_header next;
}listPosition;
extern int isEmpty(stackPosition *p);
#endif