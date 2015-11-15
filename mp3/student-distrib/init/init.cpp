#include <inc/init.h>
#include <inc/klibs/lib.h>
#include <inc/syscalls/syscalls.h>
#include <stdint.h>
#include <stddef.h>

__attribute__((used)) void init_main()
{
    /* Enable interrupts */
    sti();

    printf("Hello!\nI am the idle process!\nI am a kernel process!\nI am every other process's parent!\n");

    while(1)
    {
        printf("Starting shell ...\n");

        int32_t ret;
        const char* file = "shell";
        asm volatile(
            "movl %1, %%eax         ;\n"
            "movl %2, %%ebx         ;\n"
            "int $0x80              ;\n"
            "movl %%eax, %0         ;\n"
        :"=rm"(ret) : "i"(SYS_EXECUTE), "rm"(file) : "eax", "cc", "ebx");

        printf("Return Val: %d\n",ret);
        printf("Falling back to init.\nRe-");
    }
}
