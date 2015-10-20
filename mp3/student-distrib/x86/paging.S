# vim:ts=4 noexpandtab

#define ASM
#include <inc/x86/paging.h>

# Constant Immidiate Values only used by enable_paging:
CLEAR_PAE   =   0x00000020
SET_PG      =   0x80000000

.global enable_paging
.global global_pagedir, page_table

.text
.align 4

# enable_paging (dir_addr, pd_flags)
#     Description:
#        This function enables paging and sets CR3
#        Thus it will flush TLB.
#        C Calling Convention
#     Default effects: (set flags = 0 to use default)
#        Actually you can set the WHOLE PROCESS as not cached
#      and write-through. But we default to enable caching and
#      disable write-through on such a level.
#     Parameter:
#        pointer to head of Page Directory
#     WARNING:
#        THE POINTER MUST BE 4-KB ALIGNED!!!!
#        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
#        this pointer uses PHYSICAL address
enable_paging:
    
    # According to IA-32 manual:
    # The best order is to clear PAE first and then set PG
    # PAE is in CR4. PG is in CR0.

    movl %cr4, %eax
    andl $CLEAR_PAE, %eax
    movl %eax, %cr4

    movl %cr0, %eax
    orl  $SET_PG, %eax
    movl %eax, %cr0

    movl global_pagedir, %eax      # eax <- Pointer to Page Directory
    movl %eax, %cr3

    ret

.data

# These pointer can also point to tables in other places.
# Those table may be later allocated by kernel.
# At boot the paging tables is initialized,
#   by analyzing MEMORY LAYOUT and ALLOCATING AT THAT TIME
global_pagedir:
    .long 0

