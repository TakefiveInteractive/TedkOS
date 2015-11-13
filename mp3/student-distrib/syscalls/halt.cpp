#include "halt.h"
#include <inc/error.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/klibs/kmalloc.h>
#include <inc/x86/paging.h>
#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/palloc.h>
#include <inc/x86/desc.h>
#include <inc/x86/stacker.h>


using namespace palloc;
using arch::Stacker;
using arch::CPUArchTypes::x86;

namespace syscall_halt {

    int32_t syshalt( uint32_t retval)
    {
        thread_kinfo* prevInfo  = getCurrentThreadInfo()->pcb.prev;
        if(prevInfo == NULL){
            printf("Init Process, cannot halt!\n");
            return -1;// is init process, return -1
        }
        else//has prev
        {
            printf("Halt process!\n");
            asm volatile (
                "movl %0, %%esp         ;\n"
                "popal                  ;\n"
                "iretl                  ;\n"
                : : "rm" (prevInfo->pcb.esp0) : "cc");
        }
        return 0;
    }

}
