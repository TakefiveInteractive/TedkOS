#include <inc/init.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/palloc.h>
#include <inc/syscalls/syscalls.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/x86/real.h>
#include <inc/ui/vbe.h>
#include <stdint.h>
#include <stddef.h>

using namespace vbe;
using namespace palloc;

__attribute__((used)) void init_main()
{
    printf("Hello!\nI am the idle process!\nI am a kernel process!\nI am every other process's parent!\n");

    /* Enable interrupts */
    sti();

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
