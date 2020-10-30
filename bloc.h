#ifndef BLOC_H
#define BLOC_H
#include <stdio.h>
struct content_bloc{
    char name[512];
    char content[512][512];
};
//Pile
struct node { 
    struct content_bloc data; 
    struct node *next; 
}; 

struct list_node{
    struct node *head;
};
extern void *my_malloc(size_t size);
#endif