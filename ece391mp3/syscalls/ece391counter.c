#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

int main ()
{
    uint32_t i, cnt, max = 0;
    uint8_t buf[BUFSIZE];

    ece391_fdputs(1, (uint8_t*)"Enter the Test Number: (0): 100, (1): 10000, (2): 100000\n");
    if (-1 == (cnt = ece391_read(0, buf, BUFSIZE-1)) ) {
        ece391_fdputs(1, (uint8_t*)"Can't read the number from keyboard.\n");
     return 3;
    }
    buf[cnt] = '\0';

    if ((ece391_strlen(buf) > 2) || ((ece391_strlen(buf) == 2) && ((buf[0] < '0') || (buf[0] > '2')))) {
        ece391_fdputs(1, (uint8_t*)"Wrong Choice!\n");
        return 0;
    } else {
        switch (buf[0]) {
	        case '0':
                max = 100;
                break;
            case '1':
                max = 10000;
                break;
            case '2':
                max = 100000;
                break;
        }
    }

    for (i = 0; i < max; i++) {
        ece391_itoa(i+1, buf, 10);
        ece391_fdputs(1, buf);
        ece391_fdputs(1, (uint8_t*)"\n");
    }

    return 0;
}

