#include <stdint.h>
#include <stddef.h>

#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

// keyboard buffer size
#define BUFSIZE 128

// approximately one page of screen can have 256 chars
#define APPROX_PAGE_SIZE   256

#define termputarr(FD, ARR) {fs_write((FD), (const void*)(ARR), sizeof(ARR)/sizeof(char));}

int kreader_main ()
{
    int32_t cnt, rval;
    int32_t term, kb;
    uint8_t buf[BUFSIZE];

    printf("Open terminal\n");
    rval = fs_open("/dev/term");
    if(rval == -1)
        return -1;
    term = rval;

    printf("Open keyboard\n");
    rval = fs_open("/dev/keyb");
    if(rval == -1)
        return -1;
    kb = rval;

    termputarr (term, "Starting Kernel CMD: File Reader\n");

    while (1) {
        struct dentry_t dentry;
        uint8_t filebuf[APPROX_PAGE_SIZE] = {};

        termputarr (term, "read file> ");
        if (-1 == (cnt = fs_read (kb, (void*)buf, BUFSIZE-1))) {
            termputarr (term, "read from keyboard failed\n");
            return 3;
        }
        if (cnt > 0 && '\n' == buf[cnt - 1])
            cnt--;
        buf[cnt] = '\0';
        if ('\0' == buf[0])
            continue;

        if (cnt == 2 && buf[0] == '/' && buf[1] == '.')
        {
            int32_t fd, cnt;

            if (-1 == (fd = fs_open("/."))) {
                termputarr(term, "directory open failed\n");
                continue;
            }

            while (0 != (cnt = fs_read(fd, filebuf, sizeof(filebuf) - 1))) {
                if (-1 == cnt) {
                    termputarr(term, "directory entry read failed\n");
                    continue;
                }
                filebuf[cnt] = '\n';
                filebuf[cnt + 1] = '\0';
                printf("%s", filebuf);
            }

            continue;
        }
        int fd = fs_open((const char *)buf);
        if (-1 == fd)
        {
            termputarr (term, "no such file\n");
        }
        else
        {
            int len = 0;
            do
            {
                len = fs_read(fd, filebuf, sizeof(filebuf) - 1);
                fs_write(term, (const void*)filebuf, len);
                if (len > 0)
                {
                    termputarr(term, "\npress enter to read next block...");
                    fs_read (kb, (void*)buf, 1);
                }
            } while (len > 0);
        }
    }
    return 0;
}
