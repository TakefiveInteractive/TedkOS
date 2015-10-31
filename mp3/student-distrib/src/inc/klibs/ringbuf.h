#ifndef _KLIBS_RINGBUF_H
#define _KLIBS_RINGBUF_H

#ifndef RINGBUF_SIZE
#warning "RINGBUF_SIZE is not defined, using 128"
#define RINGBUF_SIZE        128
#else
#if(RINGBUF_SIZE <= 1)
#error "RINGBUF_SIZE should be at least 2."
#endif
#endif

#ifndef RINGBUF_TYPE
#error "RINGBUF_TYPE is not defined"
#endif

#include <stdint.h>

// This file should be used like template.
// (But in every .c the RINGBUF_SIZE can have only one value, though...

// ! Important !
// This data structure is NOT protected by lock!
// Because maybe different usage require different arrangement of locks.

// Similar to queues, but different:
//      if queue is full, nothing is pushed in
//      if ringbuf is full, it kicks out the oldest element
typedef struct 
{
    // stores the actual RINGBUF_SIZE 
    //  this value varies by .c file,
    //  thus the implementation must know it.

    int32_t _macro_ringbuf_size;

    // stores the size of the type used
    int32_t _macro_ringbuf_type_size;

    // position of next poped value.
    // -1 if empty
    int32_t head;

    // position of next pushed value.
    int32_t tail;

    // actual number of items stored.
    int32_t len;

    // !!!! data is of UNCERTAIN size !!!! MUST BE PLACED AT THE END !!!!
    RINGBUF_TYPE data[RINGBUF_SIZE];
} ringbuf_t;


#define RINGBUF_INIT(BUF)                                   \
{                                                           \
    (BUF)->head = -1;                                       \
    (BUF)->tail = 0;                                        \
    (BUF)->len = 0;                                         \
    (BUF)->_macro_ringbuf_size = RINGBUF_SIZE;              \
    (BUF)->_macro_ringbuf_type_size = sizeof(RINGBUF_TYPE); \
}

int32_t ringbuf_length(ringbuf_t* buf);

int ringbuf_is_empty(ringbuf_t* buf);

void ringbuf_push(ringbuf_t* buf, void* item);

// _front does not actually pop it.
//  If empty, returns non-zero. success <=> 0
int ringbuf_front(ringbuf_t* buf, void* result);

// This function does not do memcpy, so be careful
//      not to access the pointer after pop.
// If empty, returns null.
void* ringbuf_front_nocp(ringbuf_t* buf);

// _back does not actually pop it.
//  If empty, returns non-zero. success <=> 0
int ringbuf_back(ringbuf_t* buf, void* result);

// This function does not do memcpy, so be careful
//      not to access the pointer after pop.
// If empty, returns null.
void* ringbuf_back_nocp(ringbuf_t* buf);


void ringbuf_pop_front(ringbuf_t* buf);
void ringbuf_pop_back(ringbuf_t* buf);

#endif /* _KLIBS_RINGBUF_H */
