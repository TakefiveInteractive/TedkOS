#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/new.h>

size_t ProcessDesc::nextNewProcess = 0;
ProcessDesc *ProcessDesc::all_processes = NULL;

ProcessDesc::ProcessDesc()
{
    memset(fileDescs, 0, sizeof(fileDescs));
    mainThreadInfo = new thread_kinfo;

    mainThreadInfo->pcb.esp0 = NULL;
    mainThreadInfo->pcb.to_process = this;
    mainThreadInfo->pcb.next = NULL;
    mainThreadInfo->pcb.prev = NULL;
}

void ProcessDesc::init()
{
    if(!all_processes)
    {
        volatile uint32_t flags;
        volatile uint16_t physAddr = physPages.allocPage(1);
        void* addr = (void*)((uint32_t) 2 << 22);

        commonMemMap.add(VirtAddr(addr), PhysAddr(physAddr, PG_WRITABLE));
        currProcMemMap.add(VirtAddr(addr), PhysAddr(physAddr, PG_WRITABLE));
        if(!currProcMemMap.isLoadedToCR3(&cpu0_paging_lock))
        {
            spin_lock_irqsave(&cpu0_paging_lock, flags);
            LOAD_4MB_PAGE((uint32_t)addr >> 22, (uint32_t)physAddr << 22, PG_WRITABLE);
            spin_unlock_irqrestore(&cpu0_paging_lock, flags);
        }
        spin_lock_irqsave(&cpu0_paging_lock, flags);
        RELOAD_CR3();
        spin_unlock_irqrestore(&cpu0_paging_lock, flags);

        all_processes = (ProcessDesc*)addr;
    }
}

ProcessDesc* ProcessDesc::all()
{
    init();
    return all_processes;
}

ProcessDesc& ProcessDesc::get(size_t uniq_pid)
{
    init();
    return all_processes[uniq_pid];
}

int32_t ProcessDesc::getUniqPid()
{
    return this - all_processes;
}

size_t ProcessDesc::newProcess()
{
    init();
    return nextNewProcess++;
}

