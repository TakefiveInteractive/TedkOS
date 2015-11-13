#ifndef _INC_X86_REAL_H
#define _INC_X86_REAL_H

typedef struct
{
    int16_t di;
    int16_t si;
    uint16_t bp;
    uint16_t padding;
    int16_t bx;
    int16_t dx;
    int16_t cx;
    int16_t ax;
} pusha_t;

// legacyInt: call the specified interrupt in REAL MODE
//     This can be used to access legacy VBE functions.
// regs is both used to pass in parameter,
//     and used to receive results.
extern void legacyInt(int16_t interrupt_num, pusha_t& regs);
extern "C" void legacyInt_noLock(int16_t interrupt_num, pusha_t* regs);

extern spinlock_t legacyInt_lock;

#endif//_INC_X86_REAL_H
