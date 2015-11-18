#include <stddef.h>
#include <stdint.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/idt_table.h>
#include <inc/x86/err_handler.h>
#include <inc/syscalls/syscalls.h>
#include <inc/x86/desc.h>

/* Single bit field indicating if an error has an error code to be popped */
const unsigned long int ErrorCodeInExceptionBitField = 0x40047D00;

template<size_t index> struct VectorExtractingMetaFunc {
    static void value(void) {   // Make sure the compiler doesn't try to be too clever
        __asm__ __volatile__ (
#ifdef __OPTIMIZE__
            "push %%ebp;    \n"
            "movl %%esp, %%ebp;     \n"
#endif

            "pushl %%ecx               ;\n"
            "movl %2, %%ecx            ;\n"
            "movw %%cx, %%ds           ;\n"
            "movw %%cx, %%es           ;\n"
            "movw %%cx, %%fs           ;\n"
            "movw %%cx, %%gs           ;\n"
            "popl %%ecx                ;\n"

            "movl %0, %%esp;        \n"
            "cmpl $32, %%esp;       \n"
            "jae 1f;                \n"     // Average interrupt
            "movl %1, %%esp;        \n"
            "btl %0, %%esp;         \n"
            "leave;                 \n"
            "jz 2f;                 \n"
            "movl %%ebx, -32(%%esp);    \n" // Exception with code
            "popl %%ebx;                \n" // Pop code into EBX
            "pushal;                    \n"
            "movl -4(%%esp), %%eax;     \n"
            "movl %%eax, 16(%%esp);     \n"
            "jmp 3f;                    \n"
"1:;\n"
            "leave;                     \n"
            "pushal;                    \n"
            "pushl %0;                  \n"
            "cld;                               \n"
            "call interrupt_handler_with_number;\n"
            "addl $4, %%esp;                    \n"
            "jmp 4f;                            \n"
"2:;\n"
            "pushal;                    \n"             // Exception with no code
            "movl $0, %%ebx;            \n"
"3:;\n"
            "cld;                               \n"
            "leal -32(%%esp), %%eax;            \n"     // Load addr of top of stack at beginning of interrupt
            "pushl %%eax;                       \n"
            "pushl %%ebx;                       \n"
            "pushl %0;                          \n"
            "call exception_handler_with_number;\n"
            "addl $12, %%esp;                   \n"
"4:;\n"
            "jmp iret_sched_policy     ;\n"
            :
            : "i" (index), "i" (ErrorCodeInExceptionBitField), "i" ((uint32_t)KERNEL_DS_SEL)
            : "cc");
    }
};

vector_extracting_handler raw_interrupt_handlers[NUM_VEC];

template<int i>
class Loop {
  public:
    static inline void exec() {
        raw_interrupt_handlers[i] = VectorExtractingMetaFunc<i>::value;
        Loop<i - 1>::exec();
    }
};

template<>
class Loop<0x80> {
  public:
    static inline void exec() {
        raw_interrupt_handlers[0x80] = systemCallHandler;
        Loop<0x80 - 1>::exec();
    }
};

template<>
class Loop<0> {
  public:
    static inline void exec() {
        raw_interrupt_handlers[0] = VectorExtractingMetaFunc<0>::value;
    }
};

void init_idt_table()
{
    Loop<NUM_VEC - 1>::exec();
}

