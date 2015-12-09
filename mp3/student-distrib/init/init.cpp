#include <inc/init.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/palloc.h>
#include <inc/syscalls/syscalls.h>
#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/ece391syscall.h>
#include <stdint.h>
#include <stddef.h>
#include "draw_nikita.h"

#include <inc/klibs/arrFile.h>
#include <inc/fs/filesystem.h>
#include <inc/syscalls/filesystem_wrapper.h>
using namespace filesystem;

using scheduler::makeKThread;

volatile bool pcbLoadable = false;

void launcher(void* arg);

__attribute__((used)) void init_halted()
{
    asm volatile("1: hlt; jmp 1b;");
}

__attribute__((used)) void init_main(void* arg)
{
    File fontFile;
    auto buffer = new uint8_t[40];
    theDispatcher->open(fontFile, "testArrFile");
    theDispatcher->read(fontFile, buffer, 40);
    theDispatcher->close(fontFile);
    ArrFile *arrFile = ArrFile::getInstance((char*)buffer);
    printf("%d\n", arrFile->length());

    pcbLoadable = true;

    printf("=> I am the idle process!\n   I am a kernel process!\n   I am every other process's parent!\n");

    scheduler::enablePreemptiveScheduling();

    printf("Starting gaurd...\n");

    auto thread = makeKThread(launcher);

    ece391_dotask(thread->getProcessDesc()->getUniqPid());

    // this part is NEVER REACHED
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
