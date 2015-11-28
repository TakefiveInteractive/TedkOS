#include "vidmap.h"
#include <inc/drivers/terminal.h>
#include <inc/klibs/lib.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>

namespace syscall
{

int32_t vidmap (uint8_t** screen_start)
{
    uint32_t pde = global_cr3val[((uint32_t)screen_start) >> 22];
    if((pde & (PT_BASE | PT_USER)) || (pde | (PG_4MB_BASE & PG_USER)))
    {
        getCurrentThreadInfo()->pcb.to_process->memmap.enableVidmap(palloc::cpu0_memmap);
        *screen_start = (uint8_t*)get_vmem_phys();
        clear();
        return 0;
    }
    else return -1;
}

}

