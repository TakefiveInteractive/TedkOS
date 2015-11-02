#include <stdint.h>
#include <stddef.h>

#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

#define BUFSIZE 1024

// approximately one page of screen can have 256 chars
#define APPROX_PAGE_SIZE   256

#define termputarr(FD, ARR) {fs_write((FD), (const void*)(ARR), sizeof(ARR)/sizeof(char));}

// Currently new "instance" only needs stdin and stdout
int krtc_main (int32_t term, int32_t kb)
{
    int32_t i;
    int32_t cnt;
    int32_t rtc;
    uint8_t buf[BUFSIZE];

    termputarr (term, "Opening RTC... ");
    if (-1 == (rtc = fs_open("rtc")))
    {
        termputarr (term, "Can't open rtc.\n");
        return 3;
    }

    termputarr (term, "Reading RTC for 10 times...\n");
    
    for(i = 0; i < 10; i++)
    {
        if (-1 == (cnt = fs_read (rtc, buf, BUFSIZE-1)))
        {
            termputarr (term, "Can't read from rtc.\n");
            return 3;
        }
        termputarr(term, "TICK!\n");
    }
    termputarr (term, "Bye!\n");

    return 0;
}
