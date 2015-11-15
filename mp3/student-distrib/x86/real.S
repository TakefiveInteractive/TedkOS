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

    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    movl  %cr0, %eax
    pushl %eax

    movl  %cr4, %eax
    pushl %eax

    movl  %cr3, %eax
    pushl %eax


    # Currently the kernel stack looks like:
    # [ leave, ebp (ours), ebx, esi, edi, cr0, cr4, cr3 -> 
    # backup current Stack ESP
    movl  %esp, (cpu0_real_context)

    # Turn OFF paging
    movl  %cr0, %eax
    andl  $~REAL_MODE_PG_BIT, %eax
    movl  %eax, %cr0
    movl  $0, %eax
    movl  %eax, %cr3

    ljmp  $CS_16BIT_SEL, $REAL_MODE_CODE_BASE * 16

back_32bit_entry:
    movl  $KERNEL_DS_SEL, %esp
    movl  %esp, %ds
    movl  %esp, %ss
    movl  %esp, %fs
    movl  %esp, %gs
    movl  %esp, %es

    # return to correct stack
    movl  cpu0_real_context, %esp
    pushw %dx
    pushw %cx
    pushw %bx
    pushw %ax
    pushw %si
    pushw %di
    pushw $0                    # this will save context to cpu0_real_context
    call save_real_context
    addl  $7*2, %esp

    popl  %eax
    popl  %ecx
    popl  %edx
    movl  %eax, %cr3
    movl  %ecx, %cr4
    movl  %edx, %cr0
    popl  %edi
    popl  %esi
    popl  %ebx
    popl  %ebp

    # restore idt
    lidt  idtr_val

    # Up till now, we only have stack frame (for "leave") left
    leave
    ret

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

    # print 10 character (0x59 = 'Y')
    movw  $0x0A59, %ax
    movw  $0x0000, %bx
    movw  $10, %cx
    int   $0x10

    # ----- From now on: Working on returning to protected mode -----
    cli

    # No need to lgdt gdtr_val, because we've loaded it once.

    movl $REAL_MODE_BIT, %esp
    movl %esp, %cr0
    ljmpl $KERNEL_CS_SEL, $back_32bit_entry

halted:
    hlt
    jmp   halted
