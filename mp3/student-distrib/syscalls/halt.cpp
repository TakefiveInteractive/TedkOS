#include <inc/syscalls/halt.h>
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
#include <inc/syscalls/syscalls.h>
#include <inc/fs/filesystem.h>

using namespace filesystem;


using namespace palloc;
using arch::Stacker;
using arch::CPUArchTypes::x86;

namespace syscall { namespace halt {

int32_t syshalt(uint32_t retval)
{
    thread_kinfo* prevInfo  = getCurrentThreadInfo()->storage.pcb.execParent;
    if (prevInfo == NULL)
    {
        printf("Init Process, cannot halt!\n");
        return -1;  // is init process, return -1
    }
    else    // has prev
    {
        printf("Halt process with retval=%d!\n", retval);
        scheduler::halt(*getCurrentThreadInfo()->getPCB(), retval);
    }
    return retval;
}

} }
