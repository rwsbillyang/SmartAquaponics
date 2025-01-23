#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include "LinkedList.h"

// add data in head, returns the count of new adding
uint16_t addNode(LinkedList* pList, void *data)
{
    if(pList == NULL){
        printf("Should call &gloableList");
        return 0;
    }
    if (data == NULL)
    {
        printf("data is null");
        return 0;
    }

    Node *node = malloc(sizeof(Node));
    node->data = data;
    node->prev = NULL;

    if (pList->head == NULL)
    {
        node->next = NULL;
    }
    else
    {
        node->next = pList->head;
        pList->head->prev = node;
    }
    pList->head = node;
    pList->count++;
    return 1;
}
// delete, returns the count of deleting
uint16_t deleteNode(LinkedList* pList, void *data)
{
    if(pList == NULL){
        printf("Should call &gloableList");
        return 0;
    }
    uint16_t count = 0;
    for (Node *n = pList->head; n->next == NULL; n = n->next)
    {
        if (n->data == data)
        {
            if (n->prev == NULL && n->next == NULL)//only one element in linkedlist
            {
                pList->head = NULL;
            }
            else
            {
                if (n->next == NULL)
                { // find in tail
                    n->prev->next = NULL;
                }
                else if (n->prev == NULL)
                { // find in head
                    pList->head = n->next;
                    n->next->prev = NULL;
                }
                else
                {
                    n->next->prev = n->prev;
                    n->prev->next = n->next;
                }
            }
            free(n);
            pList->count--;
            return 1;
        }
    }
    return 0;
}

