#include <stdint.h>
#include <inc/syscalls/syscalls.h>
#include <boost/type_traits/function_traits.hpp>

extern "C"
uint32_t systemCallDispatcher(uint32_t idx, uint32_t param1, uint32_t param2, uint32_t param3)
{
    return 0;
}

void __attribute__((optimize("O0"))) systemCallHandler(void)
{
    __asm__ __volatile__ (
#ifndef __OPTIMIZE__
        "leave; \n"
#endif
        "pushl %%eax;   \n"
        "pushal;        \n"

        "pushl %%edx;   \n"
        "pushl %%ecx;   \n"
        "pushl %%ebx;   \n"
        "pushl %%eax;   \n"
        "call systemCallDispatcher; \n"

        "movl %%eax, 28 + (%%esp);  \n"
        "popal; \n"
        "popl %%eax;    \n"
        "iret;  \n"
        :
        :
        : "cc");
}

