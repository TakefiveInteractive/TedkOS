#include <inc/init.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/palloc.h>
#include <inc/syscalls/syscalls.h>
#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include "ece391syscall.h"
#include <stdint.h>
#include <stddef.h>
#include "draw_nikita.h"

volatile bool pcbLoadable = false;

void launcher(void* arg);

__attribute__((used)) void init_halted()
{
    asm volatile("1: hlt; jmp 1b;");
}

__attribute__((used)) void init_main(void* arg)
{
    pcbLoadable = true;

    printf("Hello!\nI am the idle process!\nI am a kernel process!\nI am every other process's parent!\n");

    printf("Starting gaurd...\n");

    auto thread = makeKThread(launcher, NULL);

    ece391_dotask(thread->pcb.to_process->getUniqPid());

    // this part is NEVER REACHED
    asm volatile("1: hlt; jmp 1b;");
}

__attribute__((used)) void launcher(void* arg)
{
    printf("Hello\n I am the gaurd process to ensure terminals have shells running in them\n");

    draw_nikita();

    enablePreemptiveScheduling();

    /* Enable interrupts */
    sti();

    for (;;)
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

