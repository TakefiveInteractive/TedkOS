#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>

// Smaller than zero <=> No switch.
int32_t wantToSwitchTo = -1;

target_esp0 schedDispatchDecision()
{
    if(num_hard_int() > 0)
        return NULL;
    if(wantToSwitchTo < 0)
        return NULL;
    return ProcessDesc::get(wantToSwitchTo).mainThreadInfo->pcb.esp0;
}

int32_t newPausedProcess(int32_t parentPID)
{
    thread_kinfo* parentInfo = ProcessDesc::get(parentPID).mainThreadInfo;
    int32_t uniq_pid = ProcessDesc::newProcess();
    ProcessDesc& pd = ProcessDesc::get(uniq_pid);
    pd.mainThreadInfo->pcb.prev = parentInfo;
    parentInfo->pcb.next = pd.mainThreadInfo;
}

void prepareSwitchTo(int32_t pid)
{
    wantToSwitchTo = pid;
}
