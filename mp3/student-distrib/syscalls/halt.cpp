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
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/fs/filesystem.h>

using namespace filesystem;


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
            
            // close stdin and stdout
            fs_close(0);
            fs_close(1);

            *(int32_t*)((uint32_t)prevInfo->pcb.esp0 + 7*4) = retval;
            prepareSwitchTo(prevInfo->pcb.to_process->getUniqPid());

            // asm volatile (
            //     "movl %0, %%esp         ;\n"
            //     "popal                  ;\n"
            //     "iretl                  ;\n"
            //     : : "rm" (prevInfo->pcb.esp0) : "cc");
        }
        return retval;
    }

}
