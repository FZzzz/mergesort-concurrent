#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "list.h"

llist_t* list;

int main(void)
{
    //char* arr[16];

    list = list_new();

    FILE* fptr = fopen("input.txt" , "r");

    assert(fptr);

    char line[16];

    int j = 0;
    while (fgets(line, sizeof(line), fptr)) {
        int i=0;
        while (line[i] != '\0' && i < 16)
            i++;
        line[i - 1] = '\0';
        list_add(list , line);
        j++;
    }

    fclose(fptr);

    printf("read success\n");

    FILE* fout = fopen("test.txt" , "w+");

    node_t *cur = list->head;
    while(cur) {
        fprintf(fout , "%s\n" , cur->data);
        cur = cur->next;
    }

    fclose(fout);

    return 0;
}
