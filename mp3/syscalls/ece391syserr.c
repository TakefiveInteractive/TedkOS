#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define NEG_FD -1073741823
#define BIG_FD 1073741823
#define BIG_NUM 1073741823
#define NEG_NUM -1073741823

/* call_sys
 * This function calls the system call #(num)
 * num is the syscall number to be called
 * returns 0 on success, -1 on failure
 */
int call_sys(int num)
{
	int fail;
	asm volatile
    (
        "movl %1, %%eax\n\t"
        "int $0x80"
        : "=a"(fail)
        : "g"(num)
    );
	return fail;
}

/* TEST 1 err_neg_fd
 * tries to call syscalls with file descriptor < 0
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_neg_fd(void) {
	uint8_t buf[32];
	int fail = 0;
	if (-1 != ece391_read(NEG_FD, buf, 31)) {
		fail = 2;
		ece391_fdputs (1, (uint8_t*)"read fail\n");
	}
	if (-1 != ece391_write(NEG_FD, buf, 31)) {
		fail = 2;
		ece391_fdputs (1, (uint8_t*)"write fail\n");
	}
	if (-1 != ece391_close(NEG_FD)) {
		fail = 2;
		ece391_fdputs (1, (uint8_t*)"close fail\n");
	}
	if(fail) {
		ece391_fdputs (1, (uint8_t*)"err_neg_fd: FAIL\n");
	} else {
		ece391_fdputs (1, (uint8_t*)"err_neg_fd: PASS\n");
	}
	
	return fail;
}


/* TEST 2 err_big_fd
 * tries to write to a file with file descriptor > 7
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_big_fd(void) {
	uint8_t buf[32];
	int fail = 0;
	if (-1 != ece391_read(BIG_FD, buf, 31)) {
		fail = 2;
		ece391_fdputs (1, (uint8_t*)"read fail\n");
	}
	if (-1 != ece391_write(BIG_FD, buf, 31)) {
		fail = 2;
		ece391_fdputs (1, (uint8_t*)"write fail\n");
	}
	if (-1 != ece391_close(BIG_FD)) {
		fail = 2;
		ece391_fdputs (1, (uint8_t*)"close fail\n");
	}
	if(fail) {
		ece391_fdputs (1, (uint8_t*)"err_big_fd: FAIL\n");
	} else {
		ece391_fdputs (1, (uint8_t*)"err_big_fd: PASS\n");
	}
	
	return fail;
}


/* TEST 3 err_open_lots
 * calls open correctly seven times
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_open_lots(void) {
    int32_t i, cnt = 0;
	
	// fd = 0,1 taken, so we should be able to open 6 files (2,3,4,5,6,7)
	// the last file open should fail
    for (i = 0; i < 7; i++) {
	    if (-1 == ece391_open ((uint8_t*)".")) {
			cnt++;
        }
    }
    //close all fds that were just opened.
    for(i = 2; i < 8; i++)
    {
    	ece391_close(i);
    }
    
	if (cnt == 1) {
		ece391_fdputs(1, (uint8_t*)"err_open_lots: PASS\n");
		return 0;
	} else {
		ece391_fdputs (1, (uint8_t*)"err_open_lots: FAIL\n");
		return 2;
	}
}


/* TEST 4 err_open
 * tries to open slightly incorrect filenames
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_open(void) {
	int fail = 0; // 0 if success, != 0 if fail
	// test with string that matches filename with additional character
	if (-1 != ece391_open ((uint8_t*)"helloo")) {
		ece391_fdputs (1, (uint8_t*)"'helloo' fail\n");
		fail = 2;
    }
	
	// test with string that is short of filename by one character
	if (-1 != ece391_open ((uint8_t*)"shel")) {
		ece391_fdputs (1, (uint8_t*)"'shel' fail\n");
		fail = 2;
	}
	
	// test with empty string
	if (-1 != ece391_open ((uint8_t*)"")) {
		ece391_fdputs (1, (uint8_t*)"empty string fail\n");
		fail = 2;
	}
	
	if (fail) {
		ece391_fdputs (1, (uint8_t*)"err_open: FAIL\n");
	} else {
		ece391_fdputs (1, (uint8_t*)"err_open: PASS\n");
	}
	return fail;
}


/* TEST 5 err_unopened
 * tries to close all fd.
 * tries to read and write from unopened fd's 2-7
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_unopened(void) {
	int fail = 0, i;
	uint8_t buf[32];
	// try to close all fd's. 0 and 1 are stdin and stdout. The rest
	// haven't been opened. Nothing should be able to be closed
    for (i = 0; i < 8; i++) {
	    if (-1 != ece391_close(i)) {
			ece391_fdputs (1, (uint8_t*)"close unopened or invalid fd fail\n");
			fail = 2;
        }
    }
	for (i = 2; i < 8; i++) {
	    if (-1 != ece391_read(i, buf, 31)) {
			ece391_fdputs (1, (uint8_t*)"read from unopened fd fail\n");
			fail = 2;
        }
    }
	for (i = 2; i < 8; i++) {
	    if (-1 != ece391_write(i, buf, 31)) {
			ece391_fdputs (1, (uint8_t*)"write to unopened fd fail\n");
			fail = 2;
        }
    }
	if(fail) {
		ece391_fdputs (1, (uint8_t*)"err_unopened: FAIL\n");
	} else {
		ece391_fdputs (1, (uint8_t*)"err_unopened: PASS\n");
	}
	return fail;
}

/* TEST 6 err_vidmap
 * tries to call vidmap with a NULL ptr and an address in the kernel
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
int err_vidmap(void) {
	int fail = 0; // 0 if success, != 0 if fail
	// test with NULL pointer
	if (-1 != ece391_vidmap((uint8_t **) 0x0)) {
		ece391_fdputs (1, (uint8_t*)"Null pointer fail\n");
        fail = 2;
	}
	
	if (-1 != ece391_vidmap((uint8_t **) 0x400000)) {
		ece391_fdputs (1, (uint8_t*)"Kernel pointer fail fail\n");
		fail = 2;
	}
	
	if (fail) {
		ece391_fdputs (1, (uint8_t*)"err_vidmap: FAIL\n");
	} else {
		ece391_fdputs (1, (uint8_t*)"err_vidmap: PASS\n");
	}
	
	return fail;
}

/* TEST 7 err_stdin_out
 * write to stdin read from stdout
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
 
 int err_stdin_out(void) {
	int fail = 0;
	uint8_t buf[32];
	
	if (-1 != ece391_write(0, buf, 31)) {
			ece391_fdputs (1, (uint8_t*)"write to stdin fail\n");
			fail = 2;
    }

	if (-1 != ece391_read(1, buf, 31)) {
			ece391_fdputs (1, (uint8_t*)"read from stdout fail\n");
			fail = 2;
    }
	
	if (fail) {
		ece391_fdputs (1, (uint8_t*)"err_stdin_out: FAIL\n");
	} else {
		ece391_fdputs (1, (uint8_t*)"err_stdin_out: PASS\n");
	}
 
	return fail;
 }
 
 /* TEST 8 err_syscall_num
 * call syscall 0, NEG_NUM, and BIG_NUM
 * prints "[TEST_NAME]: PASS" if behavior is EXPECTED
 *     and then returns 0
 * prints "[TEST_NAME]: FAIL" if behavior is UNEXPECTED
 *     and then returns 2
 */
 
 int err_syscall_num(void)
 {
	int fail = 0;
	
	if (-1 != call_sys(BIG_NUM)) {
		ece391_fdputs (1, (uint8_t*)"syscall 0 fail\n");
		fail = 2;
	}
	if (-1 != call_sys(NEG_NUM)) {
		ece391_fdputs (1, (uint8_t*)"big num syscall fail\n");
		fail = 2;
	}
	if (-1 != call_sys(0)) {
		ece391_fdputs (1, (uint8_t*)"neg num syscall fail\n");
		fail = 2;
	}
	
	if (fail) {
		ece391_fdputs (1, (uint8_t*)"err_syscall_num: FAIL\n");
	} else {
		ece391_fdputs (1, (uint8_t*)"err_syscall_num: PASS\n");
	}
 
	return fail;
 }


int main ()
{
	int32_t cnt, select;
    uint8_t buf[128];
	int fail = 0;

    ece391_fdputs (1, (uint8_t*)"Choose from tests 1-8. 0 to run all: ");
    if (-1 == (cnt = ece391_read (0, buf, 127))) {
        ece391_fdputs (1, (uint8_t*)"Can't read test #\n");
		return 2;
    }
	select = (int)(buf[0] - '0');
	
	switch(select) {
		case 0:
			fail += err_neg_fd();
			fail += err_big_fd();
			fail += err_open_lots();
			fail += err_open();
			fail += err_unopened();
			fail += err_vidmap();
			fail += err_stdin_out();
			fail += err_syscall_num();
			if(fail) {
				ece391_fdputs (1, (uint8_t*)"\nOverall Tests: FAIL\n");
			} else {
				ece391_fdputs (1, (uint8_t*)"\nOverall Tests: PASS\n");
			}
			return fail;
		case 1:
			return err_neg_fd();
		case 2:
			return err_big_fd();
		case 3:
			return err_open_lots();
		case 4:
			return err_open();
		case 5:
			return err_unopened();
		case 6:
			return err_vidmap();
		case 7:
			return err_stdin_out();
		case 8:
			return err_syscall_num();
		default:
			ece391_fdputs (1, (uint8_t*)"Invalid test number. Choose from tests 1-8 or 0");
			break;
	}
    return 0;
}
