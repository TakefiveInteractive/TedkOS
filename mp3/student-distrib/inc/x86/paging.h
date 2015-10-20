#ifndef _X86_PAGING_H
#define _X86_PAGING_H

// !!!! NOT FINISHED !!!!

// !!! This header is also included in ASM !!!
// !!!  Be careful to add C types and function
//  declarations in #ifndef ASM part

// Flags to enable write-through or disable cache
// To be used with: 
//      enable_paging 
//      LOAD_PAGE_DIR 
// This is effective for a WHOLE Page Directory (NOT an ENTRY)
// We usually should not use these flags!!
#define PD_DISABLE_CACHE    0x10
#define PD_WRITE_THROUGH    0x08

// The number of entires and alignment requirement
//      for Page Directory and Page Table
#define PD_NUM_ENTRIES      0x0400
#define PD_ARR_ALGIN        0x1000
#define PT_NUM_ENTRIES      0x0400
#define PT_ARR_ALGIN        0x1000

#ifndef ASM

/*************** IN CP1 we use static Page Tables ********/
/*********** Later we change by modifying pointer ********/

// global_pagedir is pointer to the currently used page DIRECTORY.
//   DO NOT directly CHANGE these pointers to switch DIR!!!
//   If you want to load another table, use LOAD_PAGE_DIR
//    OR LOAD_PAGE_TABLE. They help you modify this pointer.

// We make these pointers public so that we can use MACROs

// global_pagedir points to head of a WHOLE Page Directory. It has 1024 entries.
extern uint32_t* global_pagedir;

// LOAD_PAGE_TABLE  (PT pointer -> an ENTRY in PD)
//  This macro accepts a pointer to page table
//      THIS DOES NOT FLUSH TLB FOR YOU.
//      YOU NEED TO **FLUSH TLB**
//     Default effects: (set pd_flags = 0 to use default)
//        When set to 0, the meaning is the antonyme of all
//          PT_* FLAGS' name combined.
//     WARNING:
//        THE POINTER MUST BE 4-KB ALIGNED!!!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
//        this pointer uses PHYSICAL address
#define LOAD_PAGE_TABLE(PD_IDX, TABLE_ADDR, PT_FLAGS)   \
    {global_pagedir[(PD_IDX)] = (((uint32_t)(TABLE_ADDR)) | ((uint32_t)(PT_FLAGS)));}

//     Description:     (PD pointer -> global_pagedir)
//        This function changes the global_pagedir and sets pd_flags too
//        Thus it will NOT flush TLB.
//        C Calling Convention
//     Default effects: (set pd_flags = 0 to use default)
//        Actually you can set the WHOLE PROCESS as not cached
//      and write-through. But we default to enable caching and
//      disable write-through on such a level.
//     OUTPUT:
//        update the global var: global_pagedir
//     WARNING:
//        this pointer uses PHYSICAL address
#define INIT_GLOBAL_PAGEDIR(ACTUAL_PAGE_DIR_ADDR, PD_FLAGS)
void init_global_pagedir(uint32_t* actualPageDirAddr, uint32_t pd_flags)

// enable_paging
//     WARNING:
//        PLEASE ALLOCATE MEMORY and SET ITS VALUE BEFORE CALLING THIS FUNC!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
void* enable_paging();

// LOAD_PAGE_DIR  (PD pointer -> CR3)
//  This macro accepts a pointer to page directory
//     There is no need to flush TLB after using this function.
//     Default effects: (set pd_flags = 0 to use default)
//        Actually you can set the WHOLE PROCESS as not cached
//      and write-through. But we default to enable caching and
//      disable write-through on such a level.
//     WARNING:
//        THE POINTER MUST BE 4-KB ALIGNED!!!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
//        this pointer uses PHYSICAL address
#define LOAD_PAGE_DIR(DIR_ADDR, PD_FLAGS) \
    {asm volatile (          \
   "movl %1, %%cr3"          \
    : "="(((uint32_t)(DIR_ADDR)) | ((uint32_t)(PD_FLAGS))) : : "cc");}


// FLUSH_PAGE_TLB
//   This macro flushes the TLB but
//   still points to the same Page Directory.
//     WARNING:
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
#define FLUSH_PAGE_TLB  \
    {asm volatile (       \
    "movl %%cr3, %%eax"   \
    "movl %%eax, %%cr3"   \
    : : : "cc", "eax");}

#endif /* ASM */


#endif /* _X86_PAGING_H */
