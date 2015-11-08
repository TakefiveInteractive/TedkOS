#ifndef _X86_IDT_INIT_H
#define _X86_IDT_INIT_H

#include <inc/x86/desc.h>

#ifdef __cplusplus
extern "C" {
#endif

// This function initializes IDT table,
//		And loads the table to IDTR
extern void init_idt();

// Get the number of non-syscall interrupt
//      that has NOT 'iret' yet.
// Useful for scheduler to determine:
//      whether it's at outmost interrupt.
extern int32_t num_hard_int();

#ifdef __cplusplus
}
#endif

#endif
