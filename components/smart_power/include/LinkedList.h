#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct _Node Node;

    // linked list node
    struct _Node
    {
        void *data; // pointer which points to any data
        Node *prev;
        Node *next;
    };

    typedef struct
    {
        Node *head;
        uint32_t count;
    } LinkedList;

    /// add data in head, returns the count of new adding
    uint16_t addNode(LinkedList *pList, void *data);

    /// delete, returns the count of deleting
    uint16_t deleteNode(LinkedList *pList, void *data);

#ifdef __cplusplus
}
#endif