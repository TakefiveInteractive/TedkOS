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

int32_t sysHalt(uint32_t p, uint32_t** switch_to_esp0)
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
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t** switch_to_esp0)
    {
        return fptr(reinterpret_cast<typename function_traits<F>::arg1_type>(switch_to_esp0));
    }
};

template<>
class SystemCallArgN<2> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t** switch_to_esp0)
    {
        return fptr(reinterpret_cast<typename function_traits<F>::arg1_type>(p1),
                    reinterpret_cast<typename function_traits<F>::arg2_type>(switch_to_esp0));
    }
};

template<>
class SystemCallArgN<3> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t** switch_to_esp0)
    {
        return fptr(
                    reinterpret_cast<typename function_traits<F>::arg1_type>(p1),
                    reinterpret_cast<typename function_traits<F>::arg2_type>(p2),
                    reinterpret_cast<typename function_traits<F>::arg3_type>(switch_to_esp0)
                 );
    }
};

template<>
class SystemCallArgN<4> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t** switch_to_esp0)
    {
        return fptr(
                    reinterpret_cast<typename function_traits<F>::arg1_type>(p1),
                    reinterpret_cast<typename function_traits<F>::arg2_type>(p2),
                    reinterpret_cast<typename function_traits<F>::arg3_type>(p3),
                    reinterpret_cast<typename function_traits<F>::arg4_type>(switch_to_esp0)
                 );
    }
};

template<typename F>
static int32_t systemCallRunner(F fptr, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t** switch_to_esp0)
{
    return SystemCallArgN<function_traits<F>::arity>::run(fptr, p1, p2, p3, switch_to_esp0);
}

extern "C"
int32_t systemCallDispatcher(uint32_t idx, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t** switch_to_esp0)
{
    switch (idx)
    {
        case SYS_HALT:          return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_EXECUTE:       return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_READ:          return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_WRITE:         return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_OPEN:          return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_CLOSE:         return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_GETARGS:       return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_VIDMAP:        return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_SET_HANDLER:   return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);
        case SYS_SIGRETURN:     return systemCallRunner(sysHalt, p1, p2, p3, switch_to_esp0);

        /* Unknown syscall */
        default: return -1;
    }
}

/*
 * Basic idea about switching kernel stacks:
 *     Both syscall and PIT ensures that currPCB.esp0 points to: [ 4 byte margin, pushal, eax, iret info ]
 *          IF and ONLY IF their helper functions decide to SWITCH TO OTHER THREADs.
 *
 *     Both syscall and PIT ensures that IF their helper functions SWITCH TO OTHER THREADs,
 *          the helper function can RETURN the ESP0 to the PCB.esp0 of that thread.
 *
 *     Thus whether a SWITCH happens or NOT, AFTER changing ESP, syscall and PIT always have:
 *          [ 4 byte margin, pushal, eax, iret info]  on stack.
 *
 *     Helper functions' responsibility:
 *          1. Maintain PCB.next and PCB.prev
 *          2. Return the correct switch_to_esp0 by reading from the TARGET thread's PCB
 *          3. !! If target thread is BRAND NEW, it must INITIALIZE esp0 correctly !!
 *
 */
void __attribute__((optimize("O0"))) systemCallHandler(void)
{
    __asm__ __volatile__ (
#ifndef __OPTIMIZE__
        "leave; \n"
#endif
        "pushl %%eax;   \n"
        "pushal;        \n"

        "pushl $0;      \n"         // local var: switch_to_esp0 = NULL

        "pushl %%esp;   \n"         // pass &switch_to_esp0 as argument.
        "pushl %%edx;   \n"
        "pushl %%ecx;   \n"
        "pushl %%ebx;   \n"
        "pushl %%eax;   \n"
        "call systemCallDispatcher; \n"
        "addl $20, %%esp;           \n"

        "movl %%eax, %%ebx         ;\n"         // switch stack. save %eax to %ebx
        "movl (%%esp), %%eax       ;\n"
        "testl %%eax, %%eax        ;\n"
        "jz 1f                     ;\n"         // Jump if no kernel stack switching.
        "call getCurrentThreadInfo ;\n"
        "movl %%esp, (%%eax)       ;\n"         // Save the stack state where switch_to_esp0 has just been allocated.

        "movl (%%esp), %%ecx       ;\n"
        "movl %%ecx, %%esp         ;\n"         // Switch the kernel stack!

        "1: addl $4, %%esp         ;\n"         // Deallocate local variables (switch_to_esp0)
        "movl %%ebx, 28+0(%%esp)   ;\n"         // Set %%eax of caller context to return val of syscall.
        "popal; \n"
        "popl %%eax;    \n"
        "iret;  \n"
        :
        :
        : "cc");
}

