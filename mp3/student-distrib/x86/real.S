.global legacyInt_noLock
.global legacyInt_unreal_entry
.global legacyInt_16bit_entry

.data
#include <inc/x86/real.h>
#include <inc/x86/desc.h>

.text
.code32

legacyInt_noLock:

    cli

    pushl %ebp
    movl  %esp, %ebp

    pushl %ebx
    pushl %esi
    pushl %edi

    movl  %cr0, %eax
    pushl %eax

    movl  %cr3, %eax
    pushl %eax

    # Turn OFF paging
    movl  %cr0, %eax
    andl  $~REAL_MODE_PG_BIT, %eax
    movl  %eax, %cr0
    movl  $0, %eax
    movl  %eax, %cr3

    # Currently the kernel stack looks like:
    # [ iret, ebp, ebx, esi, edi, cr0, cr3, -> 
    ljmp  $CS_16BIT_SEL, $REAL_MODE_CODE_BASE * 16

.code16

legacyInt_16bit_entry:
    movw  $DS_16BIT_SEL, %ax
    movw  %ax, %ds
    movw  %ax, %es
    movw  %ax, %fs
    movw  %ax, %gs
    movw  %ax, %ss
    movl  $0 , %esp

    # enter real mode!
    movl  %cr0, %eax
    andl  $~REAL_MODE_BIT, %eax
    movl  %eax, %cr0
    ljmp  $REAL_MODE_CODE_BASE, $real_mode_entry - legacyInt_16bit_entry

real_mode_entry:
    movw  $REAL_MODE_DATA_BASE, %ax
    movw  %ax, %ss
    movw  %ax, %ds

    movw  $0 , %ax
    movw  %ax, %es
    movw  %ax, %fs
    movw  %ax, %gs

    STACK_FROM = 0x100
    movw  $STACK_FROM, %sp

    pushw $0        # base_high
    pushw $0        # base_low
    pushw $0x3ff    # limit

    # initializae IDT / IVT
    lidt  STACK_FROM - 4 - 2

    # switch VGA page
    movw  $0x0501, %ax 
    int   $0x10

    # print 10 character (0x59 = 'Y')
    movw  $0x0A59, %ax
    movw  $0x0100, %bx
    movw  $10, %cx
    int   $0x10

halted:
    hlt
    jmp   halted
