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

void print_control_registers(void)
{
    uint32_t cr0, cr2, cr3;
    __asm__ __volatile__(
        "mov %%cr0, %%eax\n\t"
        "mov %%eax, %0\n\t"
        "mov %%cr2, %%eax\n\t"
        "mov %%eax, %1\n\t"
        "mov %%cr3, %%eax\n\t"
        "mov %%eax, %2\n\t"
    : "=m" (cr0), "=m" (cr2), "=m" (cr3)
    : /* no input */
    : "eax"
    );
    printf("CR0 = 0x%#x, CR2 = 0x%#x, CR3 = 0x%#x\n", cr0, cr2, cr3);
}

void exception_handler(size_t vec, unsigned long int code)
{
    int have_error_code = ErrorCodeInExceptionBitField & (1 << vec);
    printf("================= WTF Exception Occurred =================\n");
    printf("0x%x => %s", vec, exception_metadata[vec].name);
    if (have_error_code)
    {
        printf(", error code is 0x%x\n", code);
    }
    else
    {
        printf("\n");
    }
    print_control_registers();
    printf("====================== END OF TRACE ======================");
    // TODO: we gotta return control to program in subsequent checkpoints
    __asm__(".1: hlt; jmp .1;");
}

