#ifndef _X86_IDT_INIT_H
#define _X86_IDT_INIT_H

#include <inc/x86/desc.h>

// This function initializes IDT table,
//		And loads the table to IDTR
extern void init_idt();

#endif
