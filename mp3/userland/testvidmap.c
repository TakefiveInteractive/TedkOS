#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "../../shared/ece391syscall.h"

int main ()
{
    uint8_t* vmem;
    if(ece391_vidmap(&vmem) != 0)
    {
        printf("cannot open video memory\n");
        return -1;
    }
    while(1)
    {
        for(int i=0; i< 80 * 24; i++)
            vmem[i*2]++;
    }
    return 0;
}

