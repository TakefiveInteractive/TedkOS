#include "vidmap.h"
#include <inc/syscalls/syscalls.h>
#include <inc/drivers/terminal.h>
#include <inc/klibs/lib.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>

namespace syscall
{


int32_t vidmap (uint8_t** screen_start)
{
    if(!validUserPointer(screen_start))
        return -1;
	getCurrentThreadInfo()->pcb.to_process->memmap.enableVidmap(palloc::cpu0_memmap);
	*screen_start = (uint8_t*)get_vmem_phys();
	clear();
	return 0;
}

}

