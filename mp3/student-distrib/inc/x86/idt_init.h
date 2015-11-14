#ifndef _X86_IDT_INIT_H
#define _X86_IDT_INIT_H

#include <inc/x86/desc.h>
#include <inc/klibs/spinlock.h>

// This function initializes IDT table,
// And loads the table to IDTR
extern void init_idt();

#ifdef __cplusplus
extern "C" {
#endif

// Get the number of non-syscall interrupt
//      that has NOT 'iret' yet.
// Useful for scheduler to determine:
//      whether it's at outmost interrupt.
extern int32_t num_nest_int();
extern spinlock_t num_nest_int_lock;
extern int32_t num_nest_int_val;

void interrupt_handler_with_number (size_t index);

#ifdef __cplusplus
}
#endif

#endif
