#include <inc/init.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/palloc.h>
#include <inc/klibs/deque.h>
#include <inc/syscalls/syscalls.h>
#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/ece391syscall.h>
#include <inc/drivers/sb16.h>
#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/pci.h>
#include "draw_nikita.h"

using scheduler::makeKThread;
using namespace pci;
using scheduler::attachThread;

volatile bool pcbLoadable = false;
volatile bool isFallbackTerm = true;

struct initHelper
{
    size_t kbClientId;
    bool isTextTerm;

    // Lock the lock before using the following fields
    spinlock_t* multitaskLock;
    int32_t* numLoadedClients;
    struct initHelper* recyclable;
};

__attribute__((used)) __attribute__((fastcall)) void launcher(void* arg);

__attribute__((used)) __attribute__((fastcall)) void init_main(void* arg)
{
    pcbLoadable = true;

    // TODO: Duang
    play_wav((char*) "duang.wav");

    printf("=> I am the idle process!\n   I am a kernel process!\n   I am every other process's parent!\n");

    initHelper* helpers = new initHelper[KeyB::KbClients::numClients];
    spinlock_t* multitaskLock = new spinlock_t;
    int32_t* numLoadedClients = new int32_t;

    *numLoadedClients = 0;
    spin_lock_init(multitaskLock);
    for(size_t i = 0; i < KeyB::KbClients::numClients; i++)
    {
        helpers[i].kbClientId       = i;
        helpers[i].isTextTerm       = (i < KeyB::KbClients::numTextTerms);
        helpers[i].multitaskLock    = multitaskLock;
        helpers[i].recyclable       = helpers;
        helpers[i].numLoadedClients = numLoadedClients;
    }

    {
        AutoSpinLock l(&KeyB::keyboard_lock);

        KeyB::getFirstTextTerm()->show();

        for(size_t i = 0; i < KeyB::KbClients::numTextTerms; i++)
        {
            auto thread = makeKThread(launcher, (void*) (&helpers[i]));
            thread->getProcessDesc()->currTerm = &(KeyB::clients.textTerms[i]);
            thread->getProcessDesc()->currTerm->setOwner(true, -1);
            thread->getProcessDesc()->currTerm->cls();
            attachThread(thread, Running);
        }
        for(size_t i = KeyB::KbClients::numTextTerms; i < KeyB::KbClients::numClients; i++)
        {
            auto thread = makeKThread(launcher, (void*) (&helpers[i]));
            thread->getProcessDesc()->currTerm = NULL;
            attachThread(thread, Running);
        }
        isFallbackTerm = false;
    }

    scheduler::enablePreemptiveScheduling();
    /* Enable interrupts */
    sti();

    asm volatile("1: hlt; jmp 1b;");
}

constexpr char TTY[] = "On TTY";

__attribute__((used)) __attribute__((fastcall)) void launcher(void* arg)
{
    // I am the guard process to ensure terminals have shells running in them!

    size_t kbClientId;
    bool isTextTerm;
    {
        initHelper* helper = (initHelper*) arg;
        AutoSpinLock l(helper->multitaskLock);
        kbClientId = helper->kbClientId;
        isTextTerm = helper->isTextTerm;
        *(helper->numLoadedClients)++;
        if(*(helper->numLoadedClients) == KeyB::KbClients::numClients)
            delete[] ((helper->recyclable));
    }

    if (isTextTerm)
    {

        //try read ata
        if(kbClientId == 0)
        {
            char *buf = new char[1024];
            auto fd = ece391_open((uint8_t*)"/dev/ata00");
            ece391_read(fd, buf, 1024);
            ece391_read(fd, buf, 1024);
            for(size_t i=0; i<1024; i++)
                printf("%x ", (uint8_t)buf[i]);
            printf("\n");
        }

        ece391_write(1, TTY, sizeof(TTY));
        char number = '0' + kbClientId;
        ece391_write(1, &number, 1);
        ece391_write(1, "\n", 1);

        for (;;)
        {
            ece391_execute((const uint8_t *)"shell");
        }
    }
    else
    {
        KeyB::clients.updateClient(kbClientId, draw_nikita());
        asm volatile("1: hlt; jmp 1b;");
    }
}
