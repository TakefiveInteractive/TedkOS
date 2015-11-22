#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/idt_init.h>
#include <inc/drivers/pit.h>

// Smaller than zero <=> No switch.
int32_t wantToSwitchTo = -1;
int32_t currentlyRunning = -1;

void enablePreemptiveScheduling()
{
    pit_init(20);   // switch every 50ms
}

void schedMakeDecision()
{
}

// Performs context switching
target_esp0 __attribute__((used)) schedDispatchExecution(target_esp0 currentESP)
{
    if (num_nest_int() > 0)
        return NULL;
    if (wantToSwitchTo < 0)
        return NULL;
    if (currentlyRunning == wantToSwitchTo)
        return NULL;

    // Firstly save current esp0 to current thread's pcb
    // Should only be saved if this is the outmost interrupt.
    getCurrentThreadInfo()->pcb.esp0 = currentESP;

    ProcessDesc& desc = ProcessDesc::get(wantToSwitchTo);

    // Switch stack
    target_esp0 ans = desc.mainThreadInfo->pcb.esp0;

    // Save new kernel stack into TSS.
    //   so that later interrupts use this new kstack
    tss.esp0 = (uint32_t)desc.mainThreadInfo->pcb.esp0;

    // Switch Page Directory
    cpu0_memmap.loadProcessMap(desc.memmap);

    currentlyRunning = wantToSwitchTo;

    // Reset dispatch decision state.
    wantToSwitchTo = -1;
    return ans;
}

int32_t newPausedProcess(int32_t parentPID)
{
    thread_kinfo* parentInfo = NULL;
    if (parentPID >= 0)
        parentInfo = ProcessDesc::get(parentPID).mainThreadInfo;
    int32_t uniq_pid = ProcessDesc::newProcess();
    ProcessDesc& pd = ProcessDesc::get(uniq_pid);
    pd.mainThreadInfo->pcb.prev = parentInfo;
    if (parentInfo)
        parentInfo->pcb.next = pd.mainThreadInfo;

    return uniq_pid;
}

// pass -1 to cancel a prepared switch.
void prepareSwitchTo(int32_t pid)
{
    wantToSwitchTo = pid;
}
