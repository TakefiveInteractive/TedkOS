#ifndef _X86_PAGING_H
#define _X86_PAGING_H

// !!!! NOT FINISHED !!!!

// !!! This header is also included in ASM !!!
// !!!  Be careful to add C types and function
//  declarations in #ifndef ASM part


// ALIGN_XKB_ADDR and **_BASE will be applied to FLAGS AUTOMATICALLY !!!!!
// DO NOT USE THEM OUTSIDE THIS HEADER !!!!!

#define ALIGN_4KB_ADDR      0xFFFFF000  // Use '&'. This will help make sure flags are not affected by address.
#define ALIGN_4MB_ADDR      0xFFC00000  // Use '&'. This will help make sure flags are not affected by address.

// APPLIES TO ALL MACRO FUNCTIONS:
//      *_BASE must be applied in all situations!!!!

// Flags to DESCRIBE actual PAGE
// This INCLUDES 4-MB Pages (Even though they are entries in PD, NOT PT)
#define PG_4MB_BASE         0x081       // Use '='
#define PG_4KB_BASE         0x001       // Use '='
#define PG_GLOABL           0x100       // Use '|'
#define PG_NOT_DIRTY        0xFFFFFFBF  // Use '&'
#define PG_NOT_ACCESSED     0xFFFFFFDF  // Use '&'
#define PG_DISABLE_CACHE    0x010       // Use '|'
#define PG_WRITE_THROUGH    0x008       // Use '|'
#define PG_USER             0x004       // Use '|'
#define PG_NOT_USER         0xFFFFFFFB  // Use '&'
#define PG_WRITABLE         0x002       // Use '|'
#define PG_READONLY         0xFFFFFFFD  // Use '&'

// Flags to DESCRIBE actual page TABLE
// This EXCLUDES 4-MB Pages
#define PT_BASE             0x001       // Use '='
#define PT_NOT_ACCESSED     0xFFFFFFDF  // Use '&'
#define PT_DISABLE_CACHE    0x010       // Use '|'
#define PT_WRITE_THROUGH    0x008       // Use '|'
#define PT_USER             0x004       // Use '|'
#define PT_NOT_USER         0xFFFFFFFB  // Use '&'
#define PT_WRITABLE         0x002       // Use '|'
#define PT_READONLY         0xFFFFFFFD  // Use '&'

// Flags to DESCRIBE page DIRECTORY
// To be used with: 
//      REDIRECT_PAGE_DIR 
// This is effective for a WHOLE Page Directory (NOT an ENTRY)
// We usually should not use these flags!!
#define PD_BASE             0x00        // Use '='
#define PD_DISABLE_CACHE    0x10        // Use '|'
#define PD_WRITE_THROUGH    0x08        // Use '|'

// The number of entires and alignment requirement
//      for Page Directory and Page Table
#define PD_NUM_ENTRIES      0x0400
#define PD_ARR_ALGIN        0x1000
#define PT_NUM_ENTRIES      0x0400
#define PT_ARR_ALGIN        0x1000

#ifndef ASM

/*************** IN CP1 we use static Page Tables ********/
/*********** Later we change by modifying pointer ********/

// global_cr3val is pointer to the currently used page DIRECTORY.
//   DO NOT directly CHANGE these pointers to switch DIR!!!
//   If you want to load another table, use REDIRECT_PAGE_DIR
//    OR LOAD_PAGE_TABLE. They help you modify this pointer.

// We make these pointers public so that we can use MACROs

// global_cr3val points to head of a WHOLE Page Directory. It has 1024 entries.
extern uint32_t* global_cr3val;

// LOAD_4KB_PAGE  (pointer will be stored IN an ENTRY in page TABLE)
//  This macro accepts a pointer to a physical 4KB page
//      THIS DOES NOT FLUSH TLB FOR YOU.
//      YOU NEED TO **FLUSH TLB**
//     WARNING:
//        TABLE_ADDR MUST HAVE FLAGS ALREADY APPLIED!!!!
//        THE POINTER PART MUST BE 4-KB ALIGNED!!!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
//        this pointer uses PHYSICAL address
#define LOAD_4KB_PAGE(PD_IDX, PT_IDX, PAGE_ADDR, FLAGS)   \
    {((uint32_t*)(global_cr3val[(PD_IDX)] & ALIGN_4KB_ADDR))[(PT_IDX)] = ((uint32_t)PG_4KB_BASE) | ((uint32_t)(PAGE_ADDR) & ALIGN_4KB_ADDR) | ((uint32_t)FLAGS);}

// LOAD_4MB_PAGE  (pointer will be stored IN an ENTRY in page DIRECTORY)
//  This macro accepts a pointer to a physical 4MB page
//      THIS DOES NOT FLUSH TLB FOR YOU.
//      YOU NEED TO **FLUSH TLB**
//     WARNING:
//        TABLE_ADDR MUST HAVE FLAGS ALREADY APPLIED!!!!
//        THE POINTER PART MUST BE 4-MB ALIGNED!!!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
//        this pointer uses PHYSICAL address
#define LOAD_4MB_PAGE(PD_IDX, PAGE_ADDR, FLAGS)   \
    {global_cr3val[(PD_IDX)] = ((uint32_t)PG_4MB_BASE) | ((uint32_t)(PAGE_ADDR) & ALIGN_4MB_ADDR) | ((uint32_t)FLAGS);}

// LOAD_PAGE_TABLE  (pointer will be stored IN an ENTRY in PD)
//  This macro accepts a pointer to page TABLE
//      THIS DOES NOT FLUSH TLB FOR YOU.
//      YOU NEED TO **FLUSH TLB**
//     WARNING:
//        TABLE_ADDR MUST HAVE FLAGS ALREADY APPLIED!!!!
//        THE POINTER PART MUST BE 4-KB ALIGNED!!!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
//        this pointer uses PHYSICAL address
#define LOAD_PAGE_TABLE(PD_IDX, TABLE_ADDR, FLAGS)   \
    {global_cr3val[(PD_IDX)] = ((uint32_t)PT_BASE) | ((uint32_t)(TABLE_ADDR) & ALIGN_4KB_ADDR) | ((uint32_t)FLAGS);}

//     Description:     (set cr3 = global_cr3val = ACTUAL_PAGE_DIR_ADDR)
//        This function changes the global_cr3val and sets pd_flags too
//        Thus it will NOT load global_cr3val to CR3 OR FLUSH TLB.
//     Default effects: (set pd_flags = 0 to use default)
//        Actually you can set the WHOLE MEMORY  as not cached
//      and write-through. But we default to enable caching and
//      disable write-through on such a level.
//     OUTPUT:
//        update the global var: global_cr3val
//     WARNING:
//        the ACTUAL_PAGE_DIR_ADDR pointer uses PHYSICAL address
#define REDIRECT_PAGE_DIR(ACTUAL_PAGE_DIR_ADDR, FLAGS) {                                 \
    global_cr3val = (uint32_t*)(((uint32_t)PD_BASE) | ((uint32_t)(ACTUAL_PAGE_DIR_ADDR) & ALIGN_4KB_ADDR) | ((uint32_t) FLAGS));  \
}

// enable_paging
//     WARNING:
//        PLEASE ALLOCATE MEMORY and SET ITS VALUE BEFORE CALLING THIS FUNC!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
void* enable_paging();

// RELOAD_CR3
//   This macro can be used to:
//      1. flushes the TLB
//      2. reload global_cr3val to CR3
//     WARNING:
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
#define RELOAD_CR3 {            \
    asm volatile (                  \
        "movl %0, %%cr3"            \
        : : "r"(global_cr3val));    \
}

#endif /* ASM */

/*
#undef PG_4MB_BASE
#undef PG_4KB_BASE
#undef PT_BASE
#undef PD_BASE
*/

#endif /* _X86_PAGING_H */
