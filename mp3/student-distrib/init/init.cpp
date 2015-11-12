#include <inc/init.h>

void init_main()
{
    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile("1: hlt; jmp 1b;");
}
