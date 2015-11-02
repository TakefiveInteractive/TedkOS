#include <stdint.h>
#include <stddef.h>

#include <inc/fops_kb.h>
#include <inc/fops_term.h>
#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/kiss_wrapper.h>

// keyboard buffer size
#define BUFSIZE 128

// approximately one page of screen can have 256 chars
#define APPROX_PAGE_SIZE   256

#define termputarr(FD, ARR) {term_write((void*)(FD), (uint8_t*)(ARR), sizeof(ARR)/sizeof(uint8_t));}

int kreader_main ()
{
    int32_t cnt, rval;
    uint8_t buf[BUFSIZE];
    rval = term_open((uint8_t*)"/dev/term");
    if(rval)
        return -1;
    rval = kb_open((uint8_t*)"/dev/kb");
    if(rval)
        return -1;
    termputarr (1, "Starting Kernel CMD: File Reader\n");

    while (1) {
        struct dentry_t dentry;
        uint8_t filebuf[APPROX_PAGE_SIZE] = {};

        termputarr (1, "read file> ");
        if (-1 == (cnt = kb_read (0, buf, BUFSIZE-1))) {
            termputarr (1, "read from keyboard failed\n");
            return 3;
        }
        if (cnt > 0 && '\n' == buf[cnt - 1])
            cnt--;
        buf[cnt] = '\0';
        if ('\0' == buf[0])
            continue;

        if (cnt == 2 && buf[0] == '/' && buf[1] == '.')
        {
            /*int i;
            for(i = 0; 0 == 0; i++)
            {
                rval = read_dentry_by_index(i, &dentry);
                if(rval)
                    break;
                termputarr(1, "File: ");
                termputarr(1, dentry.filename);
                termputarr(1, "\n");
            }*/
            int32_t fd, cnt;

            if (-1 == (fd = fs_open("/."))) {
                termputarr(1, "directory open failed\n");
                continue;
            }

            while (0 != (cnt = fs_read(fd, filebuf, sizeof(filebuf) - 1))) {
                if (-1 == cnt) {
                    termputarr(1, "directory entry read failed\n");
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
            termputarr(1, "no such file\n");
        }
        else
        {
            int len = fs_read(fd, filebuf, sizeof(filebuf) - 1);
            term_write(NULL, filebuf, len);
        }
    }
}

