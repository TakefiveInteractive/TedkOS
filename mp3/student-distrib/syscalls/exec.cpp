#include "exec.h"
#include <inc/error.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/klibs/kmalloc.h>
#include <inc/klibs/palloc.h>
#include <inc/x86/paging.h>
#include <stdint.h>
#include <stddef.h>

using namespace syscall_exec;

// Main entry to implementation of exec syscall
int32_t do_exec(const uint8_t* file)
{
    if(!is_kiss_executable(file))
        return -EINVAL;
    int32_t child_upid = newPausedProcess(getCurrentThreadInfo()->pcb.to_process->getUniqPid());

    // Load PAGE TABLEs
    /*
    uint32_t flag;
    uint32_t* pageDir = new uint32_t[0x400];
    spin_lock_irqsave(&cpu0_paging_lock, flag);

    memset(pageDir, 0, 0x1000);
    REDIRECT_PAGE_DIR(pageDir);
    LOAD_4MB_PAGE(1, 1 << 22, PG_WRITABLE);
    loadCommonPages(0);
    new4MBPage(code_page_vaddr_base >> 22, PG_USER | PG_WRITABLE, 0);
    
    spin_unlock_irqrestore(&cpu0_paging_lock, flag);
    */

    // Initialize stack and ESP

    // Load executable into memory
    kiss_loader(file);

    // Request context switch
    prepareSwitchTo(child_upid);
}
