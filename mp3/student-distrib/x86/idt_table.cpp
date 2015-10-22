#include <stddef.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/idt_table.h>
#include <inc/x86/err_handler.h>

extern "C" void interrupt_handler_with_number (size_t index, unsigned long int code);

/* Single bit field indicating if an error has an error code to be popped */
const unsigned long int ErrorCodeInExceptionBitField = 0x40047D00;

template<size_t index> struct VectorExtractingMetaFunc {
    static void __attribute__((optimize("O0"))) value(void) {   // Make sure the compiler doesn't try to be too clever
        __asm__ __volatile__ (
            "pushl %%eax;   \n"
            "movl %0, %%esp;        \n"
            "cmpl $32, %%esp;       \n"
            "jae 1f;        \n"
            "movl %1, %%eax;\n"
            "btl %%esp, %%eax;      \n"
            "jz 1f;         \n"
            "leave;         \n"
            "movl -8(%%esp), %%eax;     \n"
            "pushl %%eax;   \n"
            "movl 4(%%esp), %%eax;      \n"
            "movl %%eax, -32+4(%%esp);  \n"
            "popl %%eax;    \n"
            "addl $4, %%esp;\n"
            "pushal;        \n"
            "subl $4, %%esp;\n"
            "pushl %0;  \n"
            "jmp 2f;        \n"
"1:;\n"
            "leave;         \n"
            "movl -8(%%esp), %%eax;     \n"
            "pushal;        \n"
            "pushl $0;      \n"
            "pushl %0;  \n"
"2:;\n"
            "cld;           \n"
            "call interrupt_handler_with_number; \n"
            "addl $8, %%esp;\n"
            "popal; iret;   \n"
            :
            : "i" (index), "i" (ErrorCodeInExceptionBitField)
            : "cc");
    }
};

vector_extracting_handler raw_interrupt_handlers[NUM_VEC];

template<int i>
class LOOP {
  public:
    static inline void EXEC() {
        raw_interrupt_handlers[i] = VectorExtractingMetaFunc<i>::value;
        LOOP<i - 1>::EXEC();
    }
};

template<>
class LOOP<0> {
  public:
    static inline void EXEC() {
        raw_interrupt_handlers[0] = VectorExtractingMetaFunc<0>::value;
    }
};

void init_idt_table()
{
    LOOP<NUM_VEC - 1>::EXEC();
}
