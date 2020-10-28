#ifndef POSITIONHEADER_H
#define POSITIONHEADER_H
#include <stdio.h>
typedef struct content_bloc{
    char name[512];
    char content[512][512];
}content_bloc;
//Pile
struct node { 
    content_bloc data; 
    struct node *next; 
}; 
typedef struct node node;
struct list_node{
    node *head;
};
//extern int isEmpty(stack_content_bloc *p);
#endif