#include <stdint.h>
#include <stddef.h>

#include <inc/fops_kb.h>
#include <inc/fops_term.h>
#include <inc/x86/filesystem_wrapper.h>


// keyboard buffer size
#define BUFSIZE 128

#define termputarr(FD, ARR) {term_write((FD), (uint8_t*)(ARR), sizeof(ARR)/sizeof(uint8_t));}

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
        uint8_t filebuf[256] = {};

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

        rval = read_dentry_by_name(buf, &dentry);
        if (-1 == rval)
        {
            termputarr (1, "no such command\n");
        }
        else
        {
            size_t len = read_data(dentry.inode, 0, filebuf, sizeof(filebuf));
            term_write(1, filebuf, len);
        }
    }
}

