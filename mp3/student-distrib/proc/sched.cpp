#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/idt_init.h>

// Smaller than zero <=> No switch.
int32_t wantToSwitchTo = -1;

target_esp0 __attribute__((used)) schedDispatchDecision()
{
    if(num_hard_int() > 0)
        return NULL;
    if(wantToSwitchTo < 0)
        return NULL;

    ProcessDesc& desc = ProcessDesc::get(wantToSwitchTo);

    // Switch stack
    target_esp0 ans = desc.mainThreadInfo->pcb.esp0;

    // Switch Page Directory
    currProcMemMap = desc.memmap;
    currProcMemMap += commonMemMap;
    currProcMemMap.loadToCR3(&cpu0_paging_lock);

    // Reset dispatch decision state.
    wantToSwitchTo = -1;
    return ans;
}

int32_t newPausedProcess(int32_t parentPID)
{
    thread_kinfo* parentInfo = NULL;
    if(parentPID >= 0)
        parentInfo = ProcessDesc::get(parentPID).mainThreadInfo;
    int32_t uniq_pid = ProcessDesc::newProcess();
    ProcessDesc& pd = ProcessDesc::get(uniq_pid);
    pd.mainThreadInfo->pcb.prev = parentInfo;
    if(parentInfo)
        parentInfo->pcb.next = pd.mainThreadInfo;

    return uniq_pid;
}

// pass -1 to cancel a prepared switch.
void prepareSwitchTo(int32_t pid)
{
    wantToSwitchTo = pid;
}
