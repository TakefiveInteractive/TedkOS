#include <stddef.h>
#include <stdint.h>
#include <inc/x86/err_handler.h>
#include <inc/lib.h>

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
    { "Reserved", Abort },
    { "x87 Floating-Point Exception", Fault },
    { "Alignment Check", Fault },
    { "Machine Check", Abort },
    { "SIMD Floating-Point Exception", Fault },
    { "Virtualization Exception", Fault },
    { "Reserved #21", Abort },
    { "Reserved #22", Abort },
    { "Reserved #23", Abort },
    { "Reserved #24", Abort },
    { "Reserved #25", Abort },
    { "Reserved #26", Abort },
    { "Reserved #27", Abort },
    { "Reserved #28", Abort },
    { "Reserved #29", Abort },
    { "Security Exception", Abort },
    { "Reserved", Abort },
    { "Triple Fault", Abort }
};

void __attribute__((fastcall)) exception_handler(size_t vec, unsigned long int code)
{
    int have_error_code = ErrorCodeInExceptionBitField & (1 << vec);
    printf("WTF Exception occured! %x => %s\n", vec, exception_metadata[vec].name);
    if (have_error_code)
    {
        printf("Error code is %x\n", code);
    }
    // TODO: we gotta return control to program in subsequent checkpoints
    __asm__("hlt;");
}

