#include <inc/klibs/spinlock.h>
#include <inc/drivers/terminal.h>
#include <inc/drivers/fops_kb.h>
#include <inc/drivers/fops_term.h>
#include <inc/error.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/d2d/to_kb.h>
#include <inc/klibs/AutoSpinLock.h>

volatile int8_t ThisTerminalUsedBy[NUM_TERMINALS] = {-1};
volatile int8_t isThisTerminalWaitingForEnter[NUM_TERMINALS] = {0};

// Terminal and keyboard's "sharing" policy:
//  1. multiple program can write to the same terminal at the same time.
//      1-1. With 4 terminals, we must know WHICH TERMINAL to write to.
//      1-2. Open Terminal <=> printf <=> anyone can open it at any time
//  2. ONLY ONE PROGRAM can read from its SPECIFIC TERMINAL at one time.
//      2-1. With 4 terminals, we allow at most 4 programs reading on "keyboard"
//      2-2. Thus every terminal has its own read buffer.
//      2-3. Also: thus: Open keyboard <=> scanf <=> only 1 program can open it in same terminal at one time.

int32_t kb_write(void* fdEntity, const uint8_t* buf, int32_t nbytes)
{
    return -EFOPS;
}

int32_t kb_read(void* fdEntity, uint8_t* buf, int32_t nbytes)
{
    // Dispatcher already checked that fd is valid,
    //      thus the user must own terminal currently.
    char* cbuf = (char*) buf;

    AutoSpinLock lock(&term_lock);

    // !!! Warning: decide index using PCB, later.
    if(ThisTerminalUsedBy[0] == -1)
    {
        ThisTerminalUsedBy[0] = getCurrentThreadInfo()->pcb.to_process->getUniqPid();
        isThisTerminalWaitingForEnter[0] = 0;
    }
    else if(ThisTerminalUsedBy[0] != getCurrentThreadInfo()->pcb.to_process->getUniqPid())
    {
        return -EFOPS;
    }

    if(term_buf_pos < nbytes)
    {
        isThisTerminalWaitingForEnter[0] = 1;

        lock.waitUntil([](){ return !isThisTerminalWaitingForEnter[0]; });
    }

    volatile int32_t copylen;
    if(nbytes < term_buf_pos)
        copylen = nbytes;
    else copylen = term_buf_pos;

    // Now that we do not need term_buf_pos anymore, we clear its value here.
    term_buf_pos = 0;

    volatile int32_t i;
    for(i = 0; i < copylen; i++)
    {
        cbuf[i] = term_buf[i].displayed_char;
    }
    return i;
}

int32_t kb_open(void* fdEntity)
{
    return 0;
}

int32_t kb_close(void* fdEntity)
{
    AutoSpinLock lock(&term_lock);

    // Dispatcher already checked that fd is valid,
    //      thus the user must own terminal currently.
    ThisTerminalUsedBy[0] = -1;
    isThisTerminalWaitingForEnter[0] = 0;

    // This is required by handout. But actually clear at OPEN is enough.
    term_buf_pos = 0;
    return 0;
}

// !!!! Warning: this function does NOT lock term_lock !!!!
void term2kb_readover(int terminal_id)
{
    isThisTerminalWaitingForEnter[terminal_id] = 0;
}

