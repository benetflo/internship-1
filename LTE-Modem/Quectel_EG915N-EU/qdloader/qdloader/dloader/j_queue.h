#ifndef __J_QUEUE_H__
#define __J_QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../std_c.h"

typedef struct
{
    uint16_t wCurID;
    uint16_t dwLength;
    uint32_t dwOffset;
} Nv_Item;

#define MAX_QUEUE_SIZE 3000

typedef struct
{
    Nv_Item data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} J_Queue;

void initQueue(J_Queue *q);
int isQueueEmpty(J_Queue *q);
int isQueueFull(J_Queue *q);
int enqueue(J_Queue *q, Nv_Item p);
int dequeue(J_Queue *q, Nv_Item *p);
int j_query(J_Queue *q, int id, Nv_Item *tmp);

#endif //__J_QUEUE_H__