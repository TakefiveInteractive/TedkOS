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

    rval = fs_open((char*)"/dev/term");
    if(rval)
        return -1;
    term = rval;

    rval = fs_open((char*)"/dev/kb");
    if(rval)
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

        if (cnt == 1 && buf[0] == '.')
        {
            /*int i;
            for(i = 0; 0 == 0; i++)
            {
                rval = read_dentry_by_index(i, &dentry);
                if(rval)
                    break;
                termputarr(term, "File: ");
                termputarr(term, dentry.filename);
                termputarr(term, "\n");
            }*/
            int32_t fd, cnt;

            if (-1 == (fd = fs_open("/."))) {
                termputarr(term, "directory open failed\n");
                continue;
            }

            while (0 != (cnt = fs_read(fd, buf, sizeof(filebuf) - 1))) {
                if (-1 == cnt) {
                    termputarr(term, "directory entry read failed\n");
                    continue;
                }
                buf[cnt] = '\n';
                buf[cnt + 1] = '\0';
                printf("%s", buf);
            }

            continue;
        }
        rval = read_dentry_by_name(buf, &dentry);
        if (-1 == rval)
        {
            termputarr (term, "no such file\n");
        }
        else
        {
            size_t offset = 0;
            while(1)
            {
                size_t len = read_data(dentry.inode, offset, filebuf, sizeof(filebuf));
                if(len <= 0)
                    break;
                offset += len;
                fs_write(term, (const void*)filebuf, len);
                termputarr(term, "\npress enter to read next block...");
                fs_read (kb, (void*)buf, 1);
            }
        }
    }
}

