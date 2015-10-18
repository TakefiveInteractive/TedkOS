#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>

#include "ece391support.h"
#include "ece391syscall.h"
#include "ece391sysnum.h"


static uint32_t start_esp;
static int32_t dir_fd = -1;
static DIR* dir = NULL;


/* 
 * (copied from the real system call support)
 *
 * Rather than create a case for each number of arguments, we simplify
 * and use one macro for up to three arguments; the system calls should
 * ignore the other registers, and they're caller-saved anyway.
 */
#define DO_CALL(name,number)       \
asm volatile ("                    \
.GLOBL " #name "                  ;\
" #name ":                        ;\
        PUSHL	%EBX              ;\
	MOVL	$" #number ",%EAX ;\
	MOVL	8(%ESP),%EBX      ;\
	MOVL	12(%ESP),%ECX     ;\
	MOVL	16(%ESP),%EDX     ;\
	INT	$0x80             ;\
	CMP	$0xFFFFC000,%EAX  ;\
	JBE	1f                ;\
	MOVL	$-1,%EAX	  ;\
1:	POPL	%EBX              ;\
	RET                        \
")

/* these wrappers require no changes */
extern int32_t __ece391_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t __ece391_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t __ece391_close (int32_t fd);
void fake_function () {
DO_CALL(ece391_halt,1 /* SYS_HALT */);
DO_CALL(__ece391_read,3 /* SYS_READ */);
DO_CALL(__ece391_write,4 /* SYS_WRITE */);
DO_CALL(__ece391_close,6 /* SYS_CLOSE */);

/* Call the main() function, then halt with its return value. */

asm volatile ("                         \n\
.GLOBAL _start                          \n\
_start:                                 \n\
	MOVL	%ESP,start_esp          \n\
        CALL	main                    \n\
	PUSHL	%EAX                    \n\
	CALL	ece391_halt             \n\
");

/* end of fake container function */
}

int32_t 
ece391_execute (const uint8_t* command)
{
    int status;
    uint8_t buf[1026];
    char* args[1024];
    uint8_t* scan;
    uint32_t n_arg;

    if (1023 < ece391_strlen (command))
	return -1;
    buf[0] = '.';
    buf[1] = '/';
    ece391_strcpy (buf + 2, command);
    for (scan = buf + 2; '\0' != *scan && ' ' != *scan && '\n' != *scan; 
         scan++);
    args[0] = (char*)buf;
    n_arg = 1;
    if ('\0' != *scan) {
        *scan++ = '\0';
        /* parse arguments */
	while (1) {
	    while (' ' == *scan) scan++;
	    if ('\0' == *scan || '\n' == *scan) {
	        *scan = '\0';
		break;
	    }
	    args[n_arg++] = (char*)scan;
	    while ('\0' != *scan && ' ' != *scan && '\n' != *scan) scan++;
	    if ('\0' != *scan)
	        *scan++ = '\0';
	}
    }
    args[n_arg] = NULL;
    if (0 == fork ()) {
	execv ((char*)buf, args);
        kill (getpid (), 9);
    }
    (void)wait (&status);
    if (WIFEXITED (status))
        return WEXITSTATUS (status);
    if (9 == WTERMSIG (status))
        return -1;
    return 256;
}

int32_t 
ece391_open (const uint8_t* filename)
{
    uint32_t rval;

    if (0 == ece391_strcmp (filename, (uint8_t*)".")) {
	dir = opendir (".");
        dir_fd = open ("/dev/null", O_RDONLY);
	return dir_fd;
    }

    asm volatile ("INT $0x80" : "=a" (rval) :
		  "a" (5), "b" (filename), "c" (O_RDONLY));
    if (rval > 0xFFFFC000)
        return -1;
    return rval;
}

int32_t 
ece391_getargs (uint8_t* buf, int32_t nbytes)
{
    int32_t argc = *(uint32_t*)start_esp;
    uint8_t** argv = (uint8_t**)(start_esp + 4);
    int32_t idx, len;

    idx = 1;
    while (idx < argc) {
        len = ece391_strlen (argv[idx]);
	if (len > nbytes)
	    return -1;
        ece391_strcpy (buf, argv[idx]);
	buf += len;
	nbytes -= len;
	if (++idx >= argc)
	    break;
	if (nbytes < 1)
	    return -1;
        *buf++ = ' ';
	nbytes--;
    }
    if (nbytes < 1)
        return -1;
    *buf = '\0';
    return 0;
}

int32_t 
ece391_vidmap (uint8_t** screen_start)
{
    static int mem_fd = -1;
    void* mem_image;

    if(mem_fd == -1) {
        mem_fd = open ("/dev/mem", O_RDWR);
    }

    if ((mem_image = mmap((void*)0, 1024*1024, PROT_READ | PROT_WRITE,
                    MAP_SHARED, mem_fd, 0)) == MAP_FAILED) {
        perror ("mmap low memory");
        return -1;
    }

    *screen_start = (uint8_t*)(mem_image + 0xb8000);
    return 0;
}

int32_t 
ece391_read (int32_t fd, void* buf, int32_t nbytes)
{
    struct dirent* de;
    int32_t copied;
    uint8_t* from;
    uint8_t* to;

    if (NULL == dir || dir_fd != fd)
        return __ece391_read (fd, buf, nbytes);
    if (NULL == (de = readdir (dir)))
        return 0;
    to = buf;
    from = (uint8_t*)de->d_name;
    copied = 0;
    while ('\0' != *from) {
        *to++ = *from++;
        if (++copied == nbytes)
	    return nbytes;
	if (32 == copied)
	    return 32;
    }
    while (nbytes > copied && 32 > copied) {
        *to++ = '\0';
	copied++;
    }
    return copied;
}

int32_t 
ece391_write (int32_t fd, const void* buf, int32_t nbytes)
{
    if (NULL == dir || dir_fd != fd)
        return __ece391_write (fd, buf, nbytes);
    return -1;
}

int32_t 
ece391_close (int32_t fd)
{
    if (NULL == dir || dir_fd != fd)
        return __ece391_close (fd);
    (void)closedir (dir);
    dir = NULL;
    (void)close (dir_fd);
    dir_fd = -1;
    return 0;
}

