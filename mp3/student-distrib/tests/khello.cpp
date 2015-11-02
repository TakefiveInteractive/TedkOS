#include <stdint.h>
#include <stddef.h>

#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

#define BUFSIZE 1024

// approximately one page of screen can have 256 chars
#define APPROX_PAGE_SIZE   256

#define termputarr(FD, ARR) {fs_write((FD), (const void*)(ARR), sizeof(ARR)/sizeof(char));}

// Currently new "instance" only needs stdin and stdout
int khello_main (int32_t term, int32_t kb)
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    termputarr (term, "Hi, what's your name? ");
    if (-1 == (cnt = fs_read (kb, buf, BUFSIZE-1))) {
        termputarr (term, "Can't read name from keyboard.\n");
        return 3;
    }
    buf[cnt] = '\0';
    termputarr (term, "Hello, ");
    termputarr (term, buf);

    return 0;
}
