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

int init_que(struct Queue* _que);/* 0 success */
int full_que(struct Queue* _que);/* 1 full, else 0 */
int empty_que(struct Queue* _que);/* 1 empty, else 0 */
int push_que(struct Queue* _que, struct Message msg);/* 0 success */
int pop_que(struct Queue* _que, struct Message* msg);/* 0 success */
int peek_que(struct Queue* _que, struct Message* msg);/* 0 success */

int init_que(struct Queue* _que){/* 0 success */
    _que->fron = _que->rear = 0;
    _que->msize = 256;
    return 0;
}

int full_que(struct Queue* _que){ /* 1 full, else 0 */
    return (_que->rear+1)%_que->msize == _que->fron;
}

int empty_que(struct Queue* _que){ /* 1 empty, else 0 */
    return _que->fron == _que->rear;
}

int push_que(struct Queue* _que, struct Message msg){/* 0 success */
    if (full_que(_que))
        return -1;
    _que->buffer[_que->rear] = msg;
    _que->rear = (_que->rear+1)%_que->msize;
    return 0;
}

int pop_que(struct Queue* _que, struct Message* msg){/* 0 success */
    if (empty_que(_que))
        return -1;
    if (msg != NULL)
        *msg = _que->buffer[_que->fron];
    _que->fron = (_que->fron+1)%_que->msize;
    return 0;
}

int peek_que(struct Queue* _que, struct Message* msg){/* 0 success */
    if (empty_que(_que))
        return -1;
    *msg = _que->buffer[_que->fron];
    return 0;
}

#endif /* __QUEUE_H__ */
