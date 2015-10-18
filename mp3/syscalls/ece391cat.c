#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
    int32_t fd, cnt;
    uint8_t buf[1024];

    if (0 != ece391_getargs (buf, 1024)) {
        ece391_fdputs (1, (uint8_t*)"could not read arguments\n");
	return 3;
    }

    if (-1 == (fd = ece391_open (buf))) {
        ece391_fdputs (1, (uint8_t*)"file not found\n");
	return 2;
    }

    while (0 != (cnt = ece391_read (fd, buf, 1024))) {
        if (-1 == cnt) {
	    ece391_fdputs (1, (uint8_t*)"file read failed\n");
	    return 3;
	}
	if (-1 == ece391_write (1, buf, cnt))
	    return 3;
    }

    return 0;
}

