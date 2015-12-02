#include <stdint.h>
#include <inc/syscalls/syscalls.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <boost/type_traits/function_traits.hpp>
#include <inc/klibs/lib.h>
#include <inc/x86/desc.h>
#include <inc/proc/sched.h>
#include <inc/klibs/spinlock.h>
#include <inc/syscalls/exec.h>
#include <inc/x86/idt_init.h>
#include <inc/syscalls/halt.h>
#include "getargs.h"
#include "sbrk.h"

using namespace boost;

namespace syscall {

#define HAS_FLAG(X, FLAG) (((X) & (FLAG)) == (FLAG))

bool validUserPointer(const void* ptr)
{
    if(ptr == NULL)
        return false;

    // TODO: also add some conditions for kernel threads.
    if(getCurrentThreadInfo()->isKernel())
        return true;

    uint32_t pde = global_cr3val[((uint32_t)ptr) >> 22];
    if(HAS_FLAG(pde, PG_4MB_BASE|PG_USER))
        return true;
    else if(HAS_FLAG(pde, PT_BASE|PT_USER))
    {
        uint32_t pte = ((uint32_t*) (pde & ALIGN_4KB_ADDR))[((uint32_t)ptr & EXTRACT_PTE) >> 12];
        return HAS_FLAG(pte, PG_4KB_BASE|PG_USER);
    }
    else return false;
}

#undef HAS_FLAG

int32_t dotask(int32_t pid)
{
    if(getCurrentThreadInfo()->isKernel())
        scheduler::prepareSwitchTo(pid);
    return -1;
}

int32_t vidmap(uint8_t** ans)
{
    if(!validUserPointer(ans))
        return -EFOPS;
    *ans = getCurrentThreadInfo()->currTerm->enableVidmap();
    return 0;
}

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
    int32_t retval;
    uint32_t flag;
    spin_lock_irqsave(&num_nest_int_lock, flag);
    num_nest_int_val++;
    spin_unlock_irqrestore(&num_nest_int_lock, flag);

    switch (idx)
    {
        case SYS_HALT:          retval = systemCallRunner(halt::syshalt, p1, p2, p3); break;
        case SYS_EXECUTE:       retval = systemCallRunner(exec::sysexec, p1, p2, p3); break;
        case SYS_READ:          retval = systemCallRunner(fops::read, p1, p2, p3); break;
        case SYS_WRITE:         retval = systemCallRunner(fops::write, p1, p2, p3); break;
        case SYS_OPEN:          retval = systemCallRunner(fops::open, p1, p2, p3); break;
        case SYS_CLOSE:         retval = systemCallRunner(fops::close, p1, p2, p3); break;
        case SYS_GETARGS:       retval = systemCallRunner(exec::getargs, p1, p2, p3); break;
        case SYS_VIDMAP:        retval = systemCallRunner(halt::syshalt, p1, p2, p3); break;
        case SYS_SET_HANDLER:   retval = systemCallRunner(halt::syshalt, p1, p2, p3); break;
        case SYS_SIGRETURN:     retval = systemCallRunner(halt::syshalt, p1, p2, p3); break;
        case SYS_SBRK:          retval = systemCallRunner(sbrk::syssbrk, p1, p2, p3); break;
        case SYS_FSTAT:         retval = systemCallRunner(fops::fstat, p1, p2, p3); break;
        case SYS_LSEEK:         retval = systemCallRunner(fops::lseek, p1, p2, p3); break;
        case SYS_DOTASK:        retval = systemCallRunner(dotask, p1, p2, p3);  break;

        /* Unknown syscall */
        default: retval = -1; break;
    }

    spin_lock_irqsave(&num_nest_int_lock, flag);
    num_nest_int_val--;
    spin_unlock_irqrestore(&num_nest_int_lock, flag);
    return retval;
}

}

/*
 * Basic idea about switching kernel stacks:
 *     Both syscall and PIT ensures that currPCB.esp0 points to: [ pushal, iretl info ]
 *          IF and ONLY IF their helper functions decide to SWITCH TO OTHER THREADs.
 *
 *     IF schedDispatchExecution() wants to SWITCH TO OTHER THREADs,
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
        "call systemCallDispatcher ;\n"         // Responsibility to increment the nested counter goes to systemCallDispatcher
        "addl $16, %%esp           ;\n"
        "movl %%eax, 28+0(%%esp)   ;\n"         // Set %%eax of CALLER(old thread) context to return val of syscall.
        "jmp iret_sched_policy     ;\n"
        :
        : "i" ((uint32_t)KERNEL_DS_SEL), "i" ((uint32_t)USER_DS_SEL)
        : "cc");
}
