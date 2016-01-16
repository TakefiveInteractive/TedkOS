#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int main ()
{
    int32_t cnt, rval;
    uint8_t buf[BUFSIZE];
    ece391_fdputs (1, (uint8_t*)"Starting 391 Shell\n");

    while (1) {
        ece391_fdputs (1, (uint8_t*)"391OS> ");
	if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
	    ece391_fdputs (1, (uint8_t*)"read from keyboard failed\n");
	    return 3;
	}
	if (cnt > 0 && '\n' == buf[cnt - 1])
	    cnt--;
	buf[cnt] = '\0';
	if (0 == ece391_strcmp (buf, (uint8_t*)"exit"))
	    return 0;
	if ('\0' == buf[0])
	    continue;
	rval = ece391_execute (buf);
	if (-1 == rval)
	    ece391_fdputs (1, (uint8_t*)"no such command\n");
	else if (256 == rval)
	    ece391_fdputs (1, (uint8_t*)"program terminated by exception\n");
	else if (0 != rval)
	    ece391_fdputs (1, (uint8_t*)"program terminated abnormally\n");
    }
}

