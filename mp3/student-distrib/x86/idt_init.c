#include <stddef.h>
#include <stdint.h>
#include <inc/x86/idt_init.h>
#include <inc/x86/idt_table.h>
#include <inc/x86/err_handler.h>

//const uint32_t idtRawHandlers[256] = {
    //ARR_REPEAT_32(raw_exception_handler),
    //ARR_REPEAT_16(raw_pic_irq_handler),
    //ARR_REPEAT_80(raw_nothing_handler),
    //raw_syscall_handler,
//};

void __attribute__((fastcall)) interrupt_handler_with_number (size_t index, uint32_t code)
{
    if (index <= 0x1f)
    {
        // Exception
        exception_handler(index, code);
    }
    else if (index <= 0x20)
    {
        // PIC
    }
    else if (index <= 0x2f)
    {
        // Nothing
    }
    else if (index <= 0x7f)
    {
        // APIC
    }
    else if (index <= 0x80)
    {
        // Syscall
    }
    else if (index <= 0xee)
    {
        // More APIC
    }
    else if (index <= 0xef)
    {
         // local APIC timer
    }
    else
    {
        // SMP communication
    }
}

void init_idt(void)
{
}
