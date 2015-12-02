#include <stddef.h>
#include <stdint.h>
#include <inc/x86/err_handler.h>
#include <inc/x86/idt_init.h>
#include <inc/syscalls/syscalls.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/klibs/lib.h>
#include <inc/init.h>

enum exception_type {
    Fault,
    Trap,
    Interrupt,
    Abort
};

struct x86_exception_metadata_t {
    const char * name;
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

void __attribute__((used)) exception_handler_with_number(size_t vec, unsigned long int code, idt_stack_t *info)
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
    printf("Faulting instruction address: 0x%#x, CS: 0x%#x\n", info->EIP, info->CS);
    printf("====================== END OF TRACE ======================");

    // TODO: we gotta return control to program in subsequent checkpoints

    // Test num_nest_int() against 0 (exception handler does not increment that counter)
    //      if non-zero then a system program crashed, otherwise a user program crashed.
    if (exception_metadata[vec].type == Fault && num_nest_int() == 0)
    {
        thread_kinfo* prevInfo = pcbLoadable ? getCurrentThreadInfo()->storage.pcb.prev : NULL;
        printf("\n");
        if (prevInfo == NULL)
        {
            printf("Init Process crashed!\n");
            __asm__("1: hlt; jmp 1b;");
        }
        else    // has prev
        {
            printf("Squashing process %d ...\n", getCurrentThreadInfo()->getProcessDesc()->getPid());
            scheduler::halt(*getCurrentThreadInfo()->getPCB(), 256);
        }
    }
    else
    {
        __asm__("1: hlt; jmp 1b;");
    }

}

