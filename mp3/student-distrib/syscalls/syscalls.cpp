#include <stdint.h>
#include <inc/syscalls/syscalls.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <boost/type_traits/function_traits.hpp>
#include <inc/klibs/lib.h>
#include <inc/x86/desc.h>
#include "exec.h"
#include "halt.h"

using namespace boost;
using syscall_exec::sysexec;
using syscall_halt::syshalt;

// int32_t sysHalt(uint32_t p)
// {
//     printf("Ooo.. Halt with %d\n", p);
//     return 0;
// }

template<typename F>
F super_cast(uint32_t input)
{
    return reinterpret_cast<F>(input);
}

template<>
int32_t super_cast<int32_t>(uint32_t input)
{
    return static_cast<int32_t>(input);
}

template<unsigned N>
class SystemCallArgN { };

template<>
class SystemCallArgN<0> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr();
    }
};

template<>
class SystemCallArgN<1> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(super_cast<typename function_traits<F>::arg1_type>(p1));
    }
};

template<>
class SystemCallArgN<2> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(
                    super_cast<typename function_traits<F>::arg1_type>(p1),
                    super_cast<typename function_traits<F>::arg2_type>(p2)
                 );
    }
};

template<>
class SystemCallArgN<3> {
public:
    template<typename F>
    static int32_t run(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
    {
        return fptr(
                    super_cast<typename function_traits<F>::arg1_type>(p1),
                    super_cast<typename function_traits<F>::arg2_type>(p2),
                    super_cast<typename function_traits<F>::arg3_type>(p3)
                 );
    }
};

template<typename F>
static int32_t systemCallRunner(F fptr, uint32_t p1, uint32_t p2, uint32_t p3)
{
    return SystemCallArgN<function_traits<F>::arity>::run(fptr, p1, p2, p3);
}

extern "C"
int32_t __attribute__((used)) systemCallDispatcher(uint32_t idx, uint32_t p1, uint32_t p2, uint32_t p3)
{
    switch (idx)
    {
        case SYS_HALT:          return systemCallRunner(syshalt, p1, p2, p3);
        case SYS_EXECUTE:       return systemCallRunner(sysexec, p1, p2, p3);
        case SYS_READ:          return systemCallRunner(fs_read, p1, p2, p3);
        case SYS_WRITE:         return systemCallRunner(fs_write, p1, p2, p3);
        case SYS_OPEN:          return systemCallRunner(fs_open, p1, p2, p3);
        case SYS_CLOSE:         return systemCallRunner(fs_close, p1, p2, p3);
        case SYS_GETARGS:       return systemCallRunner(syshalt, p1, p2, p3);
        case SYS_VIDMAP:        return systemCallRunner(syshalt, p1, p2, p3);
        case SYS_SET_HANDLER:   return systemCallRunner(syshalt, p1, p2, p3);
        case SYS_SIGRETURN:     return systemCallRunner(syshalt, p1, p2, p3);

        /* Unknown syscall */
        default: return -1;
    }
}

/*
 * Basic idea about switching kernel stacks:
 *     Both syscall and PIT ensures that currPCB.esp0 points to: [ pushal, iretl info ]
 *          IF and ONLY IF their helper functions decide to SWITCH TO OTHER THREADs.
 *
 *     IF schedDispatchDecision() wants to SWITCH TO OTHER THREADs,
 *          it can RETURN the PCB.esp0 of that thread, otherwise it should return NULL.
 *          AND syscall and PIT must assign that value directly to $esp
 *
 *     Thus whether a SWITCH happens or NOT, AFTER changing ESP, syscall and PIT always have:
 *          [ pushal, iretl info ]  on stack.
 *
 *     Syscall/PIC Implementation functions' responsibility:
 *          1. Call Scheduler's functions to prepare a task switch.
 *
 *     Schduler's responsibility:
 *          1. Maintain PCB.next and PCB.prev       (.esp0 is maintained by syscall/PIT)
 *          2. Return the correct switch_to_esp0 by reading from the TARGET thread's PCB
 *          3. !! If target thread is BRAND NEW, it must INITIALIZE esp0 correctly !!
 *          4. ******* IF this is not the outmost interrupt, DO NOTHING!!!!     ************
 *
 */
void __attribute__((optimize("O0"))) systemCallHandler(void)
{
    __asm__ __volatile__ (
#ifndef __OPTIMIZE__
        "leave; \n"
#endif
        "pushal;        \n"
        "pushl %%ecx               ;\n"
        "movl %0, %%ecx            ;\n"
        "movw %%cx, %%ds           ;\n"
        "movw %%cx, %%es           ;\n"
        "movw %%cx, %%fs           ;\n"
        "movw %%cx, %%gs           ;\n"
        "popl %%ecx                ;\n"

        "pushl %%edx;   \n"
        "pushl %%ecx;   \n"
        "pushl %%ebx;   \n"
        "pushl %%eax;   \n"
        "call systemCallDispatcher ;\n"
        "movl %%eax, 28+0(%%esp)   ;\n"         // Set %%eax of CALLER(old thread) context to return val of syscall.
        "addl $16, %%esp           ;\n"
        "jmp iret_sched_policy     ;\n"
        :
        : "i" ((uint32_t)KERNEL_DS_SEL), "i" ((uint32_t)USER_DS_SEL)
        : "cc");
}
