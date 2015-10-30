#ifndef _X86_IDT_INIT_H
#define _X86_IDT_INIT_H

#include <inc/x86/desc.h>

#ifdef __cplusplus
extern "C" {
#endif

// This function initializes IDT table,
//		And loads the table to IDTR
extern void init_idt();

#ifdef __cplusplus
}
#endif

#endif
