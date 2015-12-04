#include <inc/init.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/palloc.h>
#include <inc/klibs/deque.h>
#include <inc/syscalls/syscalls.h>
#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/ece391syscall.h>
#include <stdint.h>
#include <stddef.h>
#include "draw_nikita.h"

using scheduler::makeKThread;

volatile bool pcbLoadable = false;
volatile bool isFallbackTerm = true;

void launcher(void* arg);

__attribute__((used)) void init_main(void* arg)
{
    pcbLoadable = true;

    // stores the terminal number to allocate. Must be allocated dynamically. Must be locked (after fork).
    spinlock_t* multitask_lock = new spinlock_t;
    auto termNumbers = new Deque<int>();

    spin_lock_init(multitask_lock);
    {
        AutoSpinLock l(&KeyB::keyboard_lock);
        for(size_t i=0; i<KeyB::KbClients::numTextTerms; i++)
            termNumbers->push_back(i);
    }

    printf("=> I am the idle process!\n   I am a kernel process!\n   I am every other process's parent!\n");

    scheduler::enablePreemptiveScheduling();

    ece391_fork();
    ece391_fork();

    {
        AutoSpinLock l(multitask_lock);
        size_t freeTerm = *termNumbers->back();
        termNumbers->pop_back();

        printf("Starting gaurd %d ...\n", freeTerm);

        auto thread = makeKThread(launcher);

        {
            AutoSpinLock l(&KeyB::keyboard_lock);
            thread->getProcessDesc()->currTerm = &(KeyB::clients.textTerms[freeTerm]);

            if(termNumbers->empty())
            {
                //multiple terminal instantiation complete.
                isFallbackTerm = false;
            }
        }

        ece391_dotask(thread->getProcessDesc()->getPid());
    }

    asm volatile("1: hlt; jmp 1b;");
}

__attribute__((used)) void launcher(void* arg)
{
    printf("=> I am the guard process to ensure terminals have shells running in them!\n");

    draw_nikita();

    /* Enable interrupts */
    sti();

    for (;;)
    {
        printf("Starting shell ...\n");

        int32_t ret = ece391_execute((const uint8_t *)"shell");

        printf("Return Val: %d\n",ret);
        printf("Falling back to init.\nRe-");
    }
}

