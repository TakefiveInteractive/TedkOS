#include <inc/init.h>
#include <inc/klibs/lib.h>

void init_main()
{
    printf("Hello!\nI am the idle process!\nI am a kernel process!\nI am every other process's parent!\n");
    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile("1: hlt; jmp 1b;");
}
