#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int main ()
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    ece391_fdputs (1, (uint8_t*)"Hi, what's your name? ");
    if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
        ece391_fdputs (1, (uint8_t*)"Can't read name from keyboard.\n");
        return 3;
    }
    buf[cnt] = '\0';
    ece391_fdputs (1, (uint8_t*)"Hello, ");
    ece391_fdputs (1, buf);

    return 0;
}

