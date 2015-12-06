#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "../../shared/ece391syscall.h"

int main ()
{
    volatile int i = 0;
    while(1)
    {
        i++;
    }
    return 0;
}

