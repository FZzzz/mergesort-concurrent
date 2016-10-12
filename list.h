#ifndef LLIST_H_
#define LLIST_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//typedef intptr_t val_t;

typedef struct node {
    char data[16];
    struct node *next;
} node_t;

typedef struct llist {
    node_t *head;
    uint32_t size;
} llist_t;

llist_t *list_new();
int list_add(llist_t *the_list, char* val);
void list_print(llist_t *the_list , FILE* fptr);
node_t *list_nth(llist_t *the_list, uint32_t index);

#endif
