#include <inc/klibs/spinlock.h>
#include <inc/terminal.h>
#include <inc/fops_kb.h>
#include <inc/fops_term.h>
#include <inc/error.h>

int8_t isThisTerminalInUse[NUM_TERMINALS] = {0};
int8_t isThisTerminalWaitingForEnter[NUM_TERMINALS] = {0};

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
    int32_t i;
    uint32_t flag;
    char* cbuf = (char*) buf;
    spin_lock_irqsave(& term_lock, flag);

    // If ringbuf is not empty, read the LEFTOVER
    for(i = 0; i < nbytes; i++)
    {
        term_buf_item val;
        if(ringbuf_is_empty(&term_read_buf))
            break;
        ringbuf_front(&term_read_buf, &val);
        cbuf[i] = val.displayed_char;
        ringbuf_pop_front(&term_read_buf);
        if(cbuf[i] == '\n')
        {
            spin_unlock_irqrestore(&term_lock, flag);
            return i+1;
        }
    }

    // If already read enough, return.
    if(i == nbytes)
    {
        spin_unlock_irqrestore(&term_lock, flag);
        return nbytes;
    }

    isThisTerminalWaitingForEnter[0] = 1;

    spin_unlock_irqrestore(&term_lock, flag);
    while(1)
    {
        spin_lock_irqsave(&term_lock, flag);
        if(!isThisTerminalWaitingForEnter[0])
            break;
        spin_unlock_irqrestore(&term_lock, flag);
    }

    for(; i < nbytes; i++)
    {
        term_buf_item val;
        if(ringbuf_is_empty(&term_read_buf))
            break;
        ringbuf_front(&term_read_buf, &val);
        cbuf[i] = val.displayed_char;
        ringbuf_pop_front(&term_read_buf);
        if(cbuf[i] == '\n')
        {
            spin_unlock_irqrestore(&term_lock, flag);
            return i+1;
        }
    }

    spin_unlock_irqrestore(&term_lock, flag);
    return i;
}

int32_t kb_open(void* fdEntity)
{
    int32_t retval = 0;
    uint32_t flag;
    spin_lock_irqsave(& term_lock, flag);

    // !!! Warning: decide index using PCB, later.
    if(isThisTerminalInUse[0])
        retval = -EFOPS;
    else
    {
        isThisTerminalInUse[0] = 1;
        isThisTerminalWaitingForEnter[0] = 0;

        // We ONLY need to initialize the read buffer HERE, and upon BOOTING.
        // do NOT initialize it anywhere else, except in CLOSE (required by TA)
        RINGBUF_INIT(&term_read_buf);
    }

    spin_unlock_irqrestore(&term_lock, flag);
    return retval;
}

int32_t kb_close(void* fdEntity)
{
    // Dispatcher already checked that fd is valid,
    //      thus the user must own terminal currently.
    uint32_t flag;
    spin_lock_irqsave(& term_lock, flag);
    isThisTerminalInUse[0] = 0;
    isThisTerminalWaitingForEnter[0] = 0;

    // This is required by handout. But actually clear at OPEN is enough.
    RINGBUF_INIT(&term_read_buf);
    spin_unlock_irqrestore(&term_lock, flag);
    return 0;
}

// !!!! Warning: this function does NOT lock term_lock !!!!
void term2kb_readover(int terminal_id)
{
    isThisTerminalWaitingForEnter[terminal_id] = 0;
}

