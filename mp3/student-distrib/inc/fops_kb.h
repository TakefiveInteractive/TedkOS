#ifndef _KEYBOARD_FOPS_H
#define _KEYBOARD_FOPS_H

#include <inc/terminal.h>
#include <stdint.h>
#include <stddef.h>

#ifdef RINGBUF_SIZE
#error "In the shared area between kb and term, please follow RINGBUF_SIZE=128, and do NOT override !!!"
#endif

#ifdef RINGBUF_SIZE
#error "In the shared area between kb and term, please follow RINGBUF_TYPE=term_buf_item, and do NOT override !!!"
#endif

#define RINGBUF_SIZE            TERM_BUFFER_SIZE
#define RINGBUF_TYPE            term_buf_item
#include <inc/klibs/ringbuf.h>

int32_t kb_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t kb_read(int32_t fd, void* buf, int32_t nbytes);
int32_t kb_open(const uint8_t* filename);
int32_t kb_close(int32_t fd);

#endif /* _KEYBOARD_FOPS_H */
