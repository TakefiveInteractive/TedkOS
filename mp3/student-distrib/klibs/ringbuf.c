#define RINGBUF_SIZE    2
#define RINGBUF_TYPE    uint8_t
// We use uint8_t as type because we want
//      unit to be 1 byte.
// We are the implementation and we do not set
//      the value of RINGBUF_SIZE.
//  we only need to read that value,
//      by reading ringbuf._macro_ringbuf_size

#include <inc/klibs/ringbuf.h>
#include <inc/klibs/lib.h>
#include <stddef.h>
#include <stdint.h>

int32_t ringbuf_length(ringbuf_t* buf)
{
    return buf->len;
}

int ringbuf_is_empty(ringbuf_t* buf)
{
    return buf->head == -1;
}

void ringbuf_push(ringbuf_t* buf, void* item)
{
    if(buf->head == -1)
        buf->head = buf->tail;
    else if(buf->tail == buf->head)
    {
        // the 'else' is very important.
        // In this case we clobber old value.
        buf->head = (buf->head + 1) % buf->_macro_ringbuf_size;
        buf->len--;
    }
    memcpy(buf->data + buf->tail * buf->_macro_ringbuf_type_size, item, buf->_macro_ringbuf_type_size);
    buf->tail = (buf->tail + 1) % buf->_macro_ringbuf_size;
    buf->len++;
}

// _front does not actually pop it.
//  If empty, returns non-zero. success <=> 0
int ringbuf_front(ringbuf_t* buf, void* result)
{
    if(buf->head == -1)
        return -1;
    else
    {
        memcpy(result, buf->data + buf->head * buf->_macro_ringbuf_type_size, buf->_macro_ringbuf_type_size);
        return 0;
    }
}

// This function does not do memcpy, so be careful
//      not to access the pointer after pop.
// If empty, returns null.
void* ringbuf_front_nocp(ringbuf_t* buf)
{
    if(buf->head == -1)
        return NULL;
    else
        return buf->data + buf->head * buf->_macro_ringbuf_type_size;
}

void ringbuf_pop_front(ringbuf_t* buf)
{
    if(buf->head == -1)
        return;
    buf->head = (buf->head + 1) % buf->_macro_ringbuf_size;
    buf->len--;
    if(buf->len==0)
        buf->head = -1;
}


// _back does not actually pop it.
//  If empty, returns non-zero. success <=> 0
int ringbuf_back(ringbuf_t* buf, void* result)
{
    if(buf->head == -1)
        return -1;
    else
    {
        int32_t end = (buf->tail + buf->_macro_ringbuf_size - 1) % buf->_macro_ringbuf_size;
        memcpy(result, buf->data + end * buf->_macro_ringbuf_type_size, buf->_macro_ringbuf_type_size);
        return 0;
    }
}

// This function does not do memcpy, so be careful
//      not to access the pointer after pop.
// If empty, returns null.
void* ringbuf_back_nocp(ringbuf_t* buf)
{
    if(buf->head == -1)
        return NULL;
    else
    {
        int32_t end = (buf->tail + buf->_macro_ringbuf_size - 1) % buf->_macro_ringbuf_size;
        return buf->data + end * buf->_macro_ringbuf_type_size;
    }
}

void ringbuf_pop_back(ringbuf_t* buf)
{
    if(buf->head == -1)
        return;
    buf->tail = (buf->tail + buf->_macro_ringbuf_size - 1) % buf->_macro_ringbuf_size;
    buf->len--;
    if(buf->len==0)
        buf->head = -1;
}
