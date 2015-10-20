#include <stddef.h>
#include <inc/x86/err_handler.h>

typedef void (*vector_extracting_handler)(void) __attribute__((fastcall));

template<vector_extracting_handler... args> struct ArrayHolder {
    static const vector_extracting_handler data[sizeof...(args)];
};

template<vector_extracting_handler... args>
const vector_extracting_handler ArrayHolder<args...>::data[sizeof...(args)] = { args... };

template<size_t N, template<size_t> class F, vector_extracting_handler... args>
struct generate_array_impl {
    typedef typename generate_array_impl<N-1, F, F<N>::value, args...>::result result;
};

template<template<size_t> class F, vector_extracting_handler... args>
struct generate_array_impl<0, F, args...> {
    typedef ArrayHolder<F<0>::value, args...> result;
};

template<size_t N, template<size_t> class F>
struct generate_array {
    typedef typename generate_array_impl<N-1, F>::result result;
};

extern "C" void interrupt_handler_with_number (size_t index, unsigned long int code) __attribute__((fastcall));

/* Single bit field indicating if an error has an error code to be popped */
const unsigned long int ErrorCodeInExceptionBitField = 0x40047D00;

template<size_t index> struct VectorExtractingMetaFunc {
    static void __attribute__((optimize("O0"))) __attribute__((fastcall)) value(void) {   // Make sure the compiler doesn't try to be too clever
        __asm__ __volatile__ (
            "cmpl $32, %1   \n"
            "jae 1f;        \n"
            "btl %1, %0;    \n"
            "jz 1f;         \n"
            "pushal;        \n"
            "popl %edx;     \n"
"1:;\n"
            "pushal;        \n"
            "cld;           \n"
            "movl %1, %ecx; \n"
            "call interrupt_handler_with_number; \n"
            "popal; iret;   \n"
            :
            :"r"(ErrorCodeInExceptionBitField),"r"(index)
            :
        );
    }
};

typedef generate_array<256, VectorExtractingMetaFunc>::result interrupt_table;

