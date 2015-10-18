#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define SBUFSIZE 33

int main ()
{
    int32_t fd, cnt;
    uint8_t buf[SBUFSIZE];

    if (-1 == (fd = ece391_open ((uint8_t*)"."))) {
        ece391_fdputs (1, (uint8_t*)"directory open failed\n");
        return 2;
    }

    while (0 != (cnt = ece391_read (fd, buf, SBUFSIZE-1))) {
        if (-1 == cnt) {
	        ece391_fdputs (1, (uint8_t*)"directory entry read failed\n");
	        return 3;
	    }
	    buf[cnt] = '\n';
	    if (-1 == ece391_write (1, buf, cnt + 1))
	        return 3;
    }

    return 0;
}
