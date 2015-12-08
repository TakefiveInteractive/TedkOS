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

#include <inc/drivers/sb16.h>

using scheduler::makeKThread;

volatile bool pcbLoadable = false;
volatile bool isFallbackTerm = true;

__attribute__((used)) __attribute__((fastcall)) void launcher(void* arg);

__attribute__((used)) __attribute__((fastcall)) void init_main(void* arg)
{
    pcbLoadable = true;

    // stores the terminal number to allocate. Must be allocated dynamically. Must be locked (after fork).
    spinlock_t* multitask_lock = new spinlock_t;
    auto termNumbers = new Deque<int>();

    spin_lock_init(multitask_lock);
    {
        AutoSpinLock l(&KeyB::keyboard_lock);
        for(size_t i = 0; i < KeyB::KbClients::numTextTerms; i++)
            termNumbers->push_back(i);
    }

    printf("=> I am the idle process!\n   I am a kernel process!\n   I am every other process's parent!\n");

    bool isChild = false;
    for (register size_t i = 1; i < KeyB::KbClients::numTextTerms; i++)
    {
        int val = ece391_fork();
        if (val == 0) { isChild = true; break; }    // retval 0 => I'm child
    }

    {
        AutoSpinLockKeepIF l(multitask_lock);
        size_t freeTerm = *termNumbers->back();
        termNumbers->pop_back();

        auto thread = makeKThread(launcher, (void*) freeTerm);

        {
            AutoSpinLock l(&KeyB::keyboard_lock);
            thread->getProcessDesc()->currTerm = &(KeyB::clients.textTerms[freeTerm]);
            thread->getProcessDesc()->currTerm->setOwner(true, -1);
            thread->getProcessDesc()->currTerm->cls();

            if(termNumbers->empty())
            {
                //multiple terminal instantiation complete.
                isFallbackTerm = false;
            }
        }

        ece391_dotask(thread->getProcessDesc()->getPid());
    }

    if (isChild == false)
    {
        scheduler::enablePreemptiveScheduling();
        /* Enable interrupts */
        sti();
    }
    else
    {
        // stop wasting precious scheduling time!
        scheduler::block(getCurrentThreadInfo());
    }

    asm volatile("1: hlt; jmp 1b;");
}

constexpr char TTY[] = "On TTY";

__attribute__((used)) __attribute__((fastcall)) void launcher(void* arg)
{
    // I am the guard process to ensure terminals have shells running in them!
    size_t termNo = (size_t) arg;
    ece391_write(1, TTY, sizeof(TTY));
    char number = '0' + termNo;
    ece391_write(1, &number, 1);
    ece391_write(1, "\n", 1);

    // TODO: Duang
    if (termNo == 0)
        play_wav((char*) "duang.wav");

    for (;;)
    {
        ece391_execute((const uint8_t *)"shell");
    }
}
