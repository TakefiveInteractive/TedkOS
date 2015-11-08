#include <stddef.h>
#include <stdint.h>
#include <inc/x86/err_handler.h>
#include <inc/klibs/lib.h>

enum exception_type {
    Fault,
    Trap,
    Interrupt,
    Abort
};

struct x86_exception_metadata_t {
    char * name;
    enum exception_type type;
};

static const struct x86_exception_metadata_t exception_metadata[0x21] = {
    { "Divide-by-zero", Fault },
    { "Debug", Fault },
    { "Non-maskable Interrupt", Interrupt },
    { "Breakpoint", Trap },
    { "Overflow", Trap },
    { "Bound Range Exceeded", Fault },
    { "Invalid Opcode", Fault },
    { "Device Not Available", Fault },
    { "Double Fault", Abort },
    { "Coprocessor Segment Overrun", Fault },
    { "Invalid TSS", Fault },
    { "Segment Not Present", Fault },
    { "Stack-Segment Fault", Fault },
    { "General Protection Fault", Fault },
    { "Page Fault", Fault },
    { "CPU caught cold", Abort },
    { "x87 Floating-Point Exception", Fault },
    { "Alignment Check", Fault },
    { "Machine Check", Abort },
    { "SIMD Floating-Point Exception", Fault },
    { "Virtualization Exception", Fault },
    { "Company went bankrupt", Abort },
    { "I'm a teapot", Abort },
    { "Meltdown in progress", Abort },
    { "Did not have breakfast", Abort },
    { "Cannot reverse binary tree", Abort },
    { "Unwrapping maybe type", Abort },
    { "I like monkeys", Abort },
    { "CPU on fire", Abort },
    { "CPU froze to death", Abort },
    { "Security Exception", Abort },
    { "Too much coffee", Abort },
    { "Triple Fault", Abort }
};

void exception_handler(size_t vec, unsigned long int code)
{
    int have_error_code = ErrorCodeInExceptionBitField & (1 << vec);
    printf("WTF Exception occured! 0x%x => %s\n", vec, exception_metadata[vec].name);
    if (have_error_code)
    {
        printf("Error code is 0x%x\n", code);
    }
    // TODO: we gotta return control to program in subsequent checkpoints
    __asm__(".1: hlt; jmp .1;");
}

