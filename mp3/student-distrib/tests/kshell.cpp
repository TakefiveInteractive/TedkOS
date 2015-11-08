#include <stdint.h>
#include <stddef.h>

#include <inc/tests.h>
#include <inc/fs/filesystem_wrapper.h>
#include <inc/fops_kb.h>
#include <inc/fops_term.h>
#include <inc/fs/kiss_wrapper.h>

#include <inc/klibs/lib.h>


// keyboard buffer size
#define BUFSIZE 128

#define termputarr(FD, ARR) {fs_write((uint32_t)(FD), (char*)(ARR), sizeof(ARR)/sizeof(uint8_t));}
#define iscmd(BUF, CMD) (strncmp((const char*)(BUF), (const char*)(CMD), sizeof(CMD))==0)

// fd for i/o is global
int32_t term, kb;

int exec(int (*_main)(int32_t term, int32_t kb))
{
    int subrval;
    int rval;
    int subterm, subkb;
    fs_close(term);
    fs_close(kb);

    rval = fs_open("/dev/term");
    if(rval == -1)
        return -1;
    subterm = rval;

    rval = fs_open("/dev/keyb");
    if(rval == -1)
        return -1;
    subkb = rval;

    // It's NOT that main's responsiblity to open term and kb
    subrval = _main(subterm, subkb);

    fs_close(subterm);
    fs_close(subkb);


    rval = fs_open("/dev/term");
    if(rval == -1)
        return -1;
    term = rval;

    rval = fs_open("/dev/keyb");
    if(rval == -1)
        return -1;
    kb = rval;
    
    return subrval;
}

int kshell_main ()
{
    int32_t cnt, rval;
    uint8_t buf[BUFSIZE];

    rval = fs_open("/dev/term");
    if(rval == -1)
        return -1;
    term = rval;

    rval = fs_open("/dev/keyb");
    if(rval == -1)
        return -1;
    kb = rval;

    termputarr (term, "Starting Kernel Commander\n");

    while (1) {

        termputarr (term, "kcmd> ");
        if (-1 == (cnt = fs_read (kb, buf, BUFSIZE-1))) {
            termputarr (term, "read from keyboard failed\n");
            return 3;
        }
        if (cnt > 0 && '\n' == buf[cnt - 1])
            cnt--;
        buf[cnt] = '\0';
        if ('\0' == buf[0])
            continue;

        if (iscmd(buf, "help"))
        {
            termputarr(term, "Available commands:\n");
            termputarr(term, "help, files, hello, rtc, mem\n");
        }
        else if (iscmd(buf, "mem"))
        {
            if(exec(kmem_main) < 0)
                return -1;
        }
        else if (iscmd(buf, "files"))
        {
            termputarr(term, "Calling ... \n");
            if(exec(kreader_main) < 0)
                return -1;
        }
        else if (iscmd(buf, "rtc"))
        {
            termputarr(term, "Calling ... \n");
            if(exec(krtc_main) < 0)
                return -1;
        }
        else if (iscmd(buf, "hello"))
        {
            termputarr(term, "Calling ... \n");
            if(exec(khello_main) < 0)
                return -1;
        }
        else
        {
            termputarr(term, "command not found\n");
        }
    }
}

