#include <stdint.h>
#include <inc/syscalls/syscalls.h>
#include <boost/type_traits/function_traits.hpp>
#include <inc/klibs/lib.h>

using namespace boost;

#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10

int32_t sysHalt(uint32_t p)
{
    printf("Ooo.. Halt with %d\n", p);
    return 0;
}

template<unsigned N>
class SystemCallArgN { };

template<>
class SystemCallArgN<1> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(reinterpret_cast<typename function_traits<F>::arg1_type>(p1));
    }
};

template<>
class SystemCallArgN<2> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(reinterpret_cast<typename function_traits<F>::arg1_type>(p1),
                    reinterpret_cast<typename function_traits<F>::arg2_type>(p2));
    }
};

template<>
class SystemCallArgN<3> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(
                    reinterpret_cast<typename function_traits<F>::arg1_type>(p1),
                    reinterpret_cast<typename function_traits<F>::arg2_type>(p2),
                    reinterpret_cast<typename function_traits<F>::arg3_type>(p3)
                 );
    }
};

template<typename F>
static int32_t systemCallRunner(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
{
    return SystemCallArgN<function_traits<F>::arity>::run(fptr, p1, p2, p3);
}

extern "C"
int32_t systemCallDispatcher(uint32_t idx, uint32_t p1, uint32_t p2, uint32_t p3)
{
    switch (idx)
    {
        case SYS_HALT:          return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_EXECUTE:       return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_READ:          return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_WRITE:         return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_OPEN:          return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_CLOSE:         return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_GETARGS:       return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_VIDMAP:        return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_SET_HANDLER:   return systemCallRunner(sysHalt, p1, p2, p3);
        case SYS_SIGRETURN:     return systemCallRunner(sysHalt, p1, p2, p3);

        /* Unknown syscall */
        default: return -1;
    }
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
        "addl $16, %%esp;           \n"

        "movl %%eax, 28+0(%%esp);  \n"
        "popal; \n"
        "popl %%eax;    \n"
        "iret;  \n"
        :
        :
        : "cc");
}

