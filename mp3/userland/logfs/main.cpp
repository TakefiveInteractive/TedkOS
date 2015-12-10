#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../../shared/ece391syscall.h"

const char logfsMagic[] = {'\xca', '\xfe', 'e', 'c', 'e', '3', '9', '1', 'l', 'o', 'g', 'f', 's', '\xbe', '\xef'};
const size_t logfsMagicLen = 15;
const char hdds[4][20] = {"/dev/ata00", "/dev/ata01", "/dev/ata10", "/dev/ata11"};

bool isLogFS(const uint8_t* path)
{
    printf("\t examining disk %s ... \n", (char*)path);
    uint32_t fd = ece391_open(path);
    if(fd < 0)
        return false;

    uint8_t buf[1024];
    if(0 >= ece391_read(fd, buf, 1024))
    {
        printf("read failed\n");
        ece391_close(fd);
        return false;
    }

    ece391_close(fd);

    if(0 != memcmp(buf, logfsMagic, logfsMagicLen))
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

    FILE* fd = fopen((char*)fsPath, "w");
    char buf[1024];

    size_t currBlockByte = 0;
    size_t nextToWrite = 0;

    printf("Printing content!\n");

    // ignore the first 1024 bytes
    if(fread(buf, 1, 1024, fd) > 0) while(1)
    {
        currBlockByte += 1024;
        if(fread(buf, 1, 1024, fd) <= 0)
            break;
        printf("%s", buf);

        bool shouldBreak=false;
        for(size_t i=0; i<1024; i++)
            if(buf[i]==0)
            {
                nextToWrite = i;
                shouldBreak = true;
            }
        if(shouldBreak)
            break;
    }

    printf("\n------\nPlease write something:\n");
    scanf("%s", &buf[nextToWrite]);
    printf("seek to %d\n", currBlockByte);
    fseek(fd, currBlockByte, SEEK_SET);
    fwrite(buf, 1, 1024, fd);
    fclose(fd);

    return 0;
}

