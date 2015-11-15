#ifndef _INC_X86_REAL_H
#define _INC_X86_REAL_H

#define REAL_MODE_PG_BIT        0x80000000
#define REAL_MODE_BIT           0x00000001
#define REAL_MODE_SEG_MASK      0x000ffff0
#define REAL_MODE_OFFSET_MASK   0x0000000f

#define REAL_MODE_SEG_LEN       0xffff

#define REAL_MODE_CODE_BASE     0x7c0
#define REAL_MODE_DATA_BASE     0x7d0

// If you want to use %es, use this segment
#define REAL_MODE_FREE_SEG      0x7e0

#ifndef ASM

#include <inc/x86/desc.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/lib.h>
#include <stddef.h>
#include <stdint.h>

// !!!!! DO NOT CHANGE the order, content OF THIS STRUCT !!!!!
typedef struct
{
    // 32-bit states    (DO NOT TOUCH!)
    int32_t esp;

    // real mode states (READ & WRITE)
    int16_t ax;
    int16_t bx;
    int16_t cx;
    int16_t dx;

    int16_t di;
    int16_t si;

    // States that are WRITE-ONLY
    // If you want to use %es, use REAL_MODE_FREE_SEG
    int16_t es;
} real_context_t;

// Prepare Real Mode:
//      Call this once at kernel boot, BEFORE Process Memmap is Loaded!
//      16bit codes (mainly about how to access something at 4MB)
extern void prepareRealMode();

// legacyInt: call the specified interrupt in REAL MODE
//     This can be used to access legacy VBE functions.
// regs is both used to pass in parameter,
//     and used to receive results.
// !! ONLY call this IN KERNEL, AT RING0 !!
extern void legacyInt(int16_t interrupt_num, real_context_t& regs);

// This will CLI
extern "C" void legacyInt_noLock(int16_t interrupt_num);

extern spinlock_t legacyInt_lock;

#endif//ASM

#endif//_INC_X86_REAL_H
