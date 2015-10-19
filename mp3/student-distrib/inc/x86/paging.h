#ifndef _X86_PAGING_H
#define _X86_PAGING_H

// !!!! NOT FINISHED !!!!
// pageDir pointer MUST be PROCESSED by my code to produce correct, values with CR3 flags

// !!! This header is also included in ASM !!!
// !!!  Be careful to add C types and function
//  declarations in #ifndef ASM part

// Flags to enable write-through or disable cache
// We usually should not use these flags!!
#define CR3_DISABLE_CACHE   0x10
#define CR3_WRITE_THROUGH   0x08

#ifndef ASM

// LOAD_PAGIND_DIR
//  This macro accepts a pointer to page directory
//     There is no need to flush TLB after using this function.
//     Default effects: (set flags = 0 to use default)
//        Actually you can set the WHOLE PROCESS as not cached
//      and write-through. But we default to enable caching and
//      disable write-through on such a level.
//     WARNING:
//        THE POINTER MUST BE 4-KB ALIGNED!!!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
//        this pointer uses PHYSICAL address
#define LOAD_PAGIND_DIR(PTR, FLAGS) \
    {asm volatile (          \
   "movl %1, %%cr3"          \
    : "="(((uint32_t)(PTR)) | ((uint32_t)(FLAGS))) : : "cc");}


// FLUSH_PAGING_TLB
//   This macro flushes the TLB but
//   still points to the same Page Directory.
//     WARNING:
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
#define FLUSH_PAGING_TLB  \
    {asm volatile (       \
    "movl %%cr3, %%eax"   \
    "movl %%eax, %%cr3"   \
    : : : "cc", "eax");}

// enable_paging
//     Description:
//        I wrote this as a function for easier debugging.
//        This function enables paging and sets CR3
//        Thus it will flush TLB.
//        C Calling Convention
//     Default effects: (set flags = 0 to use default)
//        Actually you can set the WHOLE PROCESS as not cached
//      and write-through. But we default to enable caching and
//      disable write-through on such a level.
//     Parameter:
//        pointer to head of Page Directory
//     WARNING:
//        THE POINTER MUST BE 4-KB ALIGNED!!!!
//        DISABLE INTERRUPT WHILE CALLING THIS FUNCTION!
//        this pointer uses PHYSICAL address
void enable_paging(void* pageDir, uint32_t flags);

#endif /* ASM */


#endif /* _X86_PAGING_H */
