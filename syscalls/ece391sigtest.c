#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024

static uint8_t charbuf;
static volatile uint8_t* badbuf = 0;
void segfault_sighandler (int signum);
void alarm_sighandler (int signum);

int main ()
{
    int32_t cnt;
    uint8_t buf[BUFSIZE];

    if (0 != ece391_getargs (buf, BUFSIZE)) {
        ece391_fdputs (1, (uint8_t*)"could not read argument\n");
	return 3;
    }

	if (buf[0] == '1') {
		ece391_fdputs(1, (uint8_t*)"Installing signal handlers\n");
		ece391_set_handler(SEGFAULT, segfault_sighandler);
		ece391_set_handler(ALARM, alarm_sighandler);
	}

    ece391_fdputs (1, (uint8_t*)"Hi, what's your name? ");
    if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
        ece391_fdputs (1, (uint8_t*)"Can't read name from keyboard.\n");
    return 3;
    }

    (*badbuf) = 1;
    buf[cnt] = '\0';
    ece391_fdputs (1, (uint8_t*)"Hello, ");
    ece391_fdputs (1, buf);
	if (charbuf == 1) {
		ece391_fdputs(1, (uint8_t*)"success\n");
	} else {
		ece391_fdputs(1, (uint8_t*)"failure\n");
	}

    return 0;
}

void
segfault_sighandler (int signum)
{
    char buf;
	uint32_t* eax;
    ece391_fdputs(1, (uint8_t*)"Segfault signal handler called, signum: ");
    switch (signum) {
        case 0: ece391_fdputs(1, (uint8_t*)"0\n"); break;
        case 1: ece391_fdputs(1, (uint8_t*)"1\n"); break;
        case 2: ece391_fdputs(1, (uint8_t*)"2\n"); break;
        case 3: ece391_fdputs(1, (uint8_t*)"3\n"); break;
        default: ece391_fdputs(1, (uint8_t*)"invalid\n"); break;
    }
    ece391_fdputs(1, (uint8_t*)"Press enter to continue...\n");
    ece391_read(0, &buf, 1);
	badbuf = &charbuf;
	eax = (uint32_t*)(&signum + 7);
	*eax = (uint32_t)&charbuf;

    ece391_fdputs(1, (uint8_t*)"Signal handler returning\n");
}

void
alarm_sighandler (int signum)
{
    ece391_fdputs(1, (uint8_t*)"Alarm signal handler called, signum: ");
    switch (signum) {
        case 0: ece391_fdputs(1, (uint8_t*)"0\n"); break;
        case 1: ece391_fdputs(1, (uint8_t*)"1\n"); break;
        case 2: ece391_fdputs(1, (uint8_t*)"2\n"); break;
        case 3: ece391_fdputs(1, (uint8_t*)"3\n"); break;
        default: ece391_fdputs(1, (uint8_t*)"invalid\n"); break;
    }
}
