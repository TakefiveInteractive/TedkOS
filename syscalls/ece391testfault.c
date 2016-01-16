#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main ()
{
	int cnt;
	uint8_t buf[1000];

    ece391_fdputs(1, (uint8_t*)"Enter the Test Number: (0): Div by 0, (1): Deref NULL, (2): ???\n");
    if (-1 == (cnt = ece391_read(0, buf, 1000-1)) ) {
        ece391_fdputs(1, (uint8_t*)"Can't read the number from keyboard.\n");
     return 3;
    }
    buf[cnt] = '\0';

    if ((ece391_strlen(buf) > 2) || ((ece391_strlen(buf) == 2) && ((buf[0] < '0') || (buf[0] > '2')))) {
        ece391_fdputs(1, (uint8_t*)"Wrong Choice!\n");
        return 0;
    } else {
    	int i;
    	int *j;
        switch (buf[0]) {
	        case '0':
				i = 1;
				i--;
				i /= i;
                break;
            case '1':
            	j = 0;
                *j = 1;
                break;
            case '2':
                break;
        }
    }

    return 0;
}

