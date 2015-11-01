#include <inc/term_fops.h>
#include <inc/error.h>

// Terminal and keyboard's "sharing" policy:
//  1. multiple program can write to the same terminal at the same time.
//      1-1. With 4 terminals, we must know WHICH TERMINAL to write to.
//      1-2. Open Terminal <=> printf <=> anyone can open it at any time
//  2. ONLY ONE PROGRAM can read from its TERMINAL at one time.
//      2-1. With 4 terminals, we allow at most 4 programs reading on "keyboard"
//      2-2. Thus every terminal has its own read buffer.
//      2-3. Also: thus: Open keyboard <=> scanf <=> only 1 program can open it in same terminal at one time.

int32_t term_write(int32_t fd, const void* buf, int32_t nbytes)
{
    int32_t i;
    for(i = 0; i < nbytes; i++)
        term_putc(buf[i]);
    return nbytes;
}

int32_t term_read(int32_t fd, void* buf, int32_t nbytes)
{
    return -EFOPS;
}

int32_t term_open(const uint8_t* filename)
{
    // Do some initializatio here !
    return 0;
}

int32_t term_close(int32_t fd)
{
    // Do some destruction here !
    // Warning: clear term_read_buffer is NOT here, but in FOPS_KB
    return 0;
}
