#include <stddef.h>
#include <inc/x86/desc.h>
#include <inc/x86/err_handler.h>

typedef void (*vector_extracting_handler)(void);

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

extern "C" void interrupt_handler_with_number (size_t index);

template<size_t index> struct VectorExtractingMetaFunc {
    static void __attribute__((optimize("O0"))) value(void) {   // Make sure the compiler doesn't try to be too clever
        __asm__("pushal; cld");
        interrupt_handler_with_number(index);
        __asm__("popal; leave; iret");      // Manually balances stack here
    }
};

const size_t count = NUM_VEC;

typedef generate_array<count, VectorExtractingMetaFunc>::result raw_interrupt_handlers;

