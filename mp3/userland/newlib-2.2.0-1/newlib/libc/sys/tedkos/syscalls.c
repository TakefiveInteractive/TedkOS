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
    for (;;);
}

int close(int file)
{
    return ece391_close(file);
}

char *__env[1] = { 0 };
char **environ = __env; /* pointer to array of char * strings that define the current environment variables */

int execve(char *name, char **argv, char **env);

int fork();

int fstat(int file, struct stat *st)
{
    return ece391_fstat(file, st);
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
    return ece391_lseek(file, ptr, dir);
}

int open(const char *name, int flags, ...)
{
    return ece391_open(name);
}

int read(int file, char *ptr, int len)
{
    return ece391_read(file, ptr, len);
}

caddr_t sbrk(int incr)
{
    return (caddr_t) ece391_sbrk(incr);
}

int stat(const char *file, struct stat *st);

clock_t times(struct tms *buf);

int unlink(char *name);

int wait(int *status);

int write(int file, char *ptr, int len)
{
    return ece391_write(file, ptr, len);
}

int gettimeofday(struct timeval *p, void *z)
{
    return -1;
}

