#include "j_queue.h"

void initQueue(J_Queue *q)
{
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

int isQueueEmpty(J_Queue *q) { return q->size == 0; }

int isQueueFull(J_Queue *q) { return q->size == MAX_QUEUE_SIZE; }

int j_query(J_Queue *q, int id, Nv_Item *tmp)
{
    for (int i = 0; i < q->size; i++)
    {
        if (q->data[i].wCurID == id)
        {
            tmp->wCurID = q->data[i].wCurID;
            tmp->dwLength = q->data[i].dwLength;
            tmp->dwOffset = q->data[i].dwOffset;
            return 1;
        }
    }
    return 0;
}

int enqueue(J_Queue *q, Nv_Item p)
{
    if (isQueueFull(q))
    {
        dprintf("J_Queue is full!\n");
        return -1;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->data[q->rear] = p;
    q->size++;
    return 0;
}

int dequeue(J_Queue *q, Nv_Item *p)
{
    if (isQueueEmpty(q))
    {
        dprintf("J_Queue is empty!\n");
        return -1;
    }
    *p = q->data[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->size--;
    return 0;
}
