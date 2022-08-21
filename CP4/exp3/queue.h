/*
    queue.h
    Author: ChongKai
*/
#ifndef __QUEUE_H__
#define __QUEUE_H__
#include <stdio.h>

struct Message{
    long dst_thread;
    char msg[256];
};

struct Queue{
    unsigned long fron, rear;
    unsigned long msize;
    struct Message buffer[256];
};

int init_que(struct Queue* _que);/* Always 0 */
int full_que(struct Queue* _que);/* 1 for full, else 0 */
int empty_que(struct Queue* _que);/* 1 for empty, else 0 */
int push_que(struct Queue* _que, struct Message msg);/* 0 for success, else -1 */
int pop_que(struct Queue* _que, struct Message* msg);/* 0 for success, else -1 */
int peek_que(struct Queue* _que, struct Message* msg);/* 0 for success, else -1 */

int init_que(struct Queue* _que){
    _que->fron = _que->rear = 0;
    _que->msize = 256;
    return 0;
}

int full_que(struct Queue* _que){
    return (_que->rear+1)%_que->msize == _que->fron;
}

int empty_que(struct Queue* _que){
    return _que->fron == _que->rear;
}

int push_que(struct Queue* _que, struct Message msg){
    if (full_que(_que))
        return -1;
    _que->buffer[_que->rear] = msg;
    _que->rear = (_que->rear+1)%_que->msize;
    return 0;
}

int pop_que(struct Queue* _que, struct Message* msg){
    if (empty_que(_que))
        return -1;
    if (msg != NULL)
        *msg = _que->buffer[_que->fron];
    _que->fron = (_que->fron+1)%_que->msize;
    return 0;
}

int peek_que(struct Queue* _que, struct Message* msg){
    if (empty_que(_que))
        return -1;
    *msg = _que->buffer[_que->fron];
    return 0;
}

int free_que(struct Queue* _que){
    return 0;
}

#endif /* __QUEUE_H__ */
