#ifndef POSITIONHEADER_H
#define POSITIONHEADER_H
#include <stdio.h>
typedef struct pos_header{
    off_t positon;
    int nb_block;
}pos_header;
#endif