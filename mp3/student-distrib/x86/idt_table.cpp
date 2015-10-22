#include <stddef.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/idt_table.h>
#include <inc/x86/err_handler.h>

extern "C" void interrupt_handler_with_number (size_t index, unsigned long int code) __attribute__((fastcall));

/* Single bit field indicating if an error has an error code to be popped */
const unsigned long int ErrorCodeInExceptionBitField = 0x40047D00;

template<size_t index> struct VectorExtractingMetaFunc {
    static void __attribute__((optimize("O0"))) __attribute__((fastcall)) value(void) {   // Make sure the compiler doesn't try to be too clever
        __asm__ __volatile__ (
            "movl $index, %%ebp;"
            "cmpl $32, %%ebp;  \n"
            "jae 1f;        \n"
            "btl %%ebp, $ErrorCodeInExceptionBitField;    \n"
            "jz 1f;         \n"
            "leave;         \n"
            "popl %%edx;    \n"
            "pushal;        \n"
"1:;\n"
            "leave;         \n"
            "pushal;        \n"
            "cld;           \n"
            "movl $index, %%ecx;\n"
            "call interrupt_handler_with_number; \n"
            "popal; iret;   \n"
            :
            :
            : "cc");
    }
};

const int ARRAY_SIZE = NUM_VEC;

template <int N, int I=N-1>
class Table : public Table<N, I-1>
{
public:
    static const vector_extracting_handler dummy;
};

template <int N>
class Table<N, 0>
{
public:
    static const vector_extracting_handler dummy;
    static vector_extracting_handler array[N];
};

template <int N, int I>
const vector_extracting_handler Table<N, I>::dummy =
    Table<N, 0>::array[I] = (vector_extracting_handler)((unsigned long int)VectorExtractingMetaFunc<I>::value
    + ((unsigned long int)Table<N, I-1>::dummy * 0));

template <int N>
const vector_extracting_handler Table<N, 0>::dummy =
    Table<N, 0>::array[0] = (vector_extracting_handler)VectorExtractingMetaFunc<0>::value;

template <int N>
vector_extracting_handler Table<N, 0>::array[N];

template class Table<ARRAY_SIZE>;

const vector_extracting_handler * raw_interrupt_handlers = Table<ARRAY_SIZE>::array;

