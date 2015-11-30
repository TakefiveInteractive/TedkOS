#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/idt_init.h>
#include <inc/x86/stacker.h>
#include <inc/drivers/pit.h>
#include <inc/drivers/kbterm.h>

using arch::Stacker;
using arch::CPUArchTypes::x86;

// Smaller than zero <=> No switch.
volatile int32_t wantToSwitchTo = -1;
volatile int32_t currentlyRunning = -1;

void setTSS(const thread_pcb& pcb)
{
    if(pcb.isKernelThread)
        tss.esp0 = (uint32_t)pcb.esp0 + (8 + 3) * 4;
    else tss.esp0 = (uint32_t)pcb.esp0 + (8 + 5) * 4;
}

void enablePreemptiveScheduling()
{
    pit_init(20);   // switch every 50ms
}

void schedMakeDecision()
{
    // Put whatever scheduling policy here
    // call "prepareSwitchTo" to schedule a context switch
}

// Performs context switching
target_esp0 __attribute__((used)) schedDispatchExecution(target_esp0 currentESP)
{
    if (num_nest_int() > 0)
        return NULL;
    if (wantToSwitchTo < 0)
    {
        if(!getCurrentThreadInfo()->pcb.isKernelThread)
        {
            getCurrentThreadInfo()->pcb.esp0 = (target_esp0)((uint32_t) getCurrentThreadInfo() + THREAD_KSTACK_SIZE - 4);
            tss.esp0 = (uint32_t) getCurrentThreadInfo()->pcb.esp0;
        }
        return NULL;
    }
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
    setTSS(desc.mainThreadInfo->pcb);

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

    // TODO: FIXME: Currently all processes are binded to terminal 0
    pd.currTerm = KeyB::getFirstTextTerm();

    return uniq_pid;
}

// pass -1 to cancel a prepared switch.
void prepareSwitchTo(int32_t pid)
{
    wantToSwitchTo = pid;
}

thread_kinfo* makeKThread(kthread_entry entry, void* arg)
{
    // should have loaded flags using cli_and_save or pushfl
    uint32_t flags = 0;
    int32_t child_upid = newPausedProcess(-1);

    if(child_upid < 0)
    {
        printf("Error: Out of PIDs\n");
        return NULL;
    }

    ProcessDesc& proc = ProcessDesc::get(child_upid);

    // Here we do NOT use any more memory than PCB & kstack.
    // Because no stack exchange happens for kthread during interrupts.

    // Initialize stack and ESP
    // compatible with x86 32-bit iretl. KTHREAD mode.
    // always no error code on stack before iretl
    Stacker<x86> kstack((uint32_t)proc.mainThreadInfo->kstack + THREAD_KSTACK_SIZE - 1);

    // EFLAGS: Clear V8086 , Clear Trap, Clear Nested Tasks.
    // Set Interrupt Enable Flag. IOPL = 3
    kstack << ((flags & (~0x24100)) | 0x3200);

    kstack << (uint32_t) KERNEL_CS_SEL;
    kstack << (uint32_t) entry;

    pushal_t regs;
    regs.esp = (uint32_t) kstack.getESP();
    regs.ebp = 0;
    regs.eax = (uint32_t) arg;
    regs.ebx = regs.ecx = regs.edx = 0;
    regs.edi = regs.esi = 0;

    kstack << regs;

    proc.mainThreadInfo->pcb.esp0 = (target_esp0)kstack.getESP();
    proc.mainThreadInfo->pcb.isKernelThread = 1;

    return proc.mainThreadInfo;
}

void forceStartThread(union _thread_kinfo* thread)
{
    cli();
    if(!thread)
        return;
    if(!cpu0_memmap.isStarted())
    {
        // Update fallback_txt_vmem for fallback putc() and clear()
        Maybe<uint32_t> vmemBase = virtOfPage0();
        char* vmemPage = NULL;
        if(vmemBase)
            vmemPage = (char*)(+vmemBase);
        else
        {
            printf("Fail to allocate virtual mem space for VMEM\n");
            asm volatile("1: hlt; jmp 1b;");
        }
        fallback_txt_vmem = vmemPage + PRE_INIT_VIDEO;

        cpu0_memmap.start();
    }
    cpu0_memmap.loadProcessMap(thread->pcb.to_process->memmap);

    // refresh TSS so that later interrupts use this new kstack
    tss.esp0 = (uint32_t)thread->pcb.esp0;

    asm volatile (
        "movl %0, %%esp         ;\n"
        "popal                  ;\n"
        "iretl                  ;\n"
        : : "rm" (thread->pcb.esp0) : "cc");
    // This asm block changes everything but gcc should not worry about them.
}

// Note that it's not recommended to get current thread's regs,
//    and such actions will return NULL
pushal_t* getRegs(thread_kinfo* thread)
{
    if(thread == getCurrentThreadInfo())
        return NULL;
    if(!thread)
        return NULL;
    return (pushal_t*)thread->pcb.esp0;
}

