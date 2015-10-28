#ifndef _KLIBS_QUEUE_H
#define _KLIBS_QUEUE_H

// Please #define _QUEUE_SIZE before including

#ifndef _KLIBS_QUEUE_SIZE
#warning "_QUEUE_SIZE is not defined. Default to 128"
#define _KLIBS_QUEUE_SIZE     128
#endif

typedef struct
{
    void* data[_KLIBS_QUEUE_SIZE];
    int front;                      // The place to pop next value. -1 if empty.
    int end;                        // The place to store next pushed value
} queue_t;

void init_queue(queue_t* q)
{
    q->front = -1;
    q->end = 0;
}

void push_queue(queue_t* q)
{
    ;
}

#endif /*_KLIBS_QUEUE_H*/
