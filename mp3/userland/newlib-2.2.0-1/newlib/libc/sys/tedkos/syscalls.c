/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <../../../../../syscall/ece391syscall.h>

void _exit(int code)
{
    ece391_halt(code);
}

int close(int file)
{
    ece391_close(file);
}

char *__env[1] = { 0 };
char **environ = __env; /* pointer to array of char * strings that define the current environment variables */

int execve(char *name, char **argv, char **env);

int fork();

int fstat(int file, struct stat *st)
{
    // TODO: impl
    st->st_mode = S_IFCHR;
    return 0;
}

int getpid();

int isatty(int file)
{
    // TODO: impl
    return 1;
}

int kill(int pid, int sig);

int link(char *old, char *new);

int lseek(int file, int ptr, int dir)
{
    // TODO: impl
    return 0;
}

int open(const char *name, int flags, ...)
{
    return ece391_open(filename);
}

int read(int file, char *ptr, int len)
{
    return ece391_read(file, ptr, len);
}

caddr_t sbrk(int incr)
{
    extern char _end;   /* Defined by the linker */
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}

int stat(const char *file, struct stat *st);

clock_t times(struct tms *buf);

int unlink(char *name);

int wait(int *status);

int write(int file, char *ptr, int len)
{
    return ece391_write(file, ptr, len);
}

int gettimeofday(struct timeval *p, struct timezone *z)
{
    return -1;
}

