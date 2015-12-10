#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../../shared/ece391syscall.h"

const char logfsMagic[] = "\0ca\0feece391logfs\0be\0ef";
const char hdds[4][20] = {"/dev/ata00", "/dev/ata01", "/dev/ata10", "/dev/ata11"};

bool isLogFS(const uint8_t* path)
{
    printf("\t examining disk %s ... \n", (char*)path);
    uint32_t fd = ece391_open(path);
    if(fd < 0)
        return false;

    uint8_t buf[1024];
    if(0 != ece391_read(fd, buf, 1024))
    {
        ece391_close(fd);
        return false;
    }

    ece391_close(fd);

    if(0 != memcmp(buf, logfsMagic, strlen(logfsMagic)))
        return false;
    else 
    {
        printf("\t\t\tFOUND logfs !\n");
        return true;
    }
}

extern "C" int main ()
{
    uint8_t fsPath[1024];

    bool no_fs = false;

    if (0 != ece391_getargs (fsPath, 1024))
        no_fs = true;
    else if(!isLogFS(fsPath))
        no_fs = true;

    if(no_fs)
    {
        printf("invalid arguments\n");
        printf("Trying to find logfs hard disk ...\n");
        for(size_t i = 0; i < 4; i++)
        {
            memcpy(fsPath, hdds[i], strlen(hdds[i])+1);
            if(isLogFS(fsPath))
            {
                no_fs = false;
                break;
            }
        }
        if(no_fs)
        {
            printf("logfs is not present in system!\n");
            return 1;
        }
    }

    return 0;
}

