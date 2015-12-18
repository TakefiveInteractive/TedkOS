#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/x86/desc_interrupts.h>
#include <inc/x86/idt_init.h>
#include <inc/x86/stacker.h>
#include <inc/drivers/pit.h>
#include <inc/drivers/kbterm.h>
#include <inc/klibs/deque.h>
#include <inc/init.h>

using arch::Stacker;
using arch::CPUArchTypes::x86;

namespace scheduler {

spinlock_t sched_lock = SPINLOCK_UNLOCKED;

// Smaller than zero <=> No switch.
volatile int32_t wantToSwitchTo = -1;
volatile int32_t currentlyRunning = -1;
volatile bool preemptiveEnabled = false;

Deque<thread_kinfo*> *rrQueue = NULL;

void init()
{
    sched_lock = SPINLOCK_UNLOCKED;
    rrQueue = new Deque<thread_kinfo*>();
    wantToSwitchTo = -1;
    currentlyRunning = -1;
    preemptiveEnabled = false;
}

void attachThread(thread_kinfo* pcb, ThreadState newState)
{
    AutoSpinLock l(&sched_lock);
    pcb->getPCB()->runState = newState;
    rrQueue->push_back(pcb);
}

void setTSS(const thread_pcb& pcb)
{
    tss.esp0 = (uint32_t)pcb.esp0;
}

void enablePreemptiveScheduling()
{
    AutoSpinLock l(&sched_lock);
    if (preemptiveEnabled) return;
    pit_init(50);   // switch every 20ms
    preemptiveEnabled = true;
}

void makeDecisionNoLock()
{
    if(rrQueue->empty())
        return;

    // find the process to run
    thread_kinfo* next = *(rrQueue->front());
    while(next->getPCB()->runState != Running)
    {
        rrQueue->pop_front();
        rrQueue->push_back(next);
        next = *(rrQueue->front());
    }

    // schedule to run this process
    wantToSwitchTo = next->getProcessDesc()->getPid();

    rrQueue->pop_front();
    rrQueue->push_back(next);
}

void makeDecision()
{
    AutoSpinLock l(&sched_lock);
    makeDecisionNoLock();
}

Pid newDetachedProcess(int32_t parentPID, ProcessType processType)
{
    AutoSpinLock l(&sched_lock);
    ProcessDesc& pd = ProcessDesc::newProcess(parentPID, processType);

    // TODO: FIXME: Currently all processes are binded to terminal 0
    pd.currTerm = (isFallbackTerm || parentPID < 0) ? KeyB::getFirstTextTerm() : ProcessDesc::get(parentPID).currTerm;

    return pd.getPid();
}

// pass -1 to cancel a prepared switch.
void prepareSwitchTo(int32_t pid)
{
    wantToSwitchTo = pid;
}

thread_kinfo* makeKThread(kthread_entry entry)
{
    return makeKThread(entry, nullptr);
}

thread_kinfo* makeKThread(kthread_entry entry, void* arg)
{
    // should have loaded flags using cli_and_save or pushfl
    uint32_t flags = 0;
    int32_t child_upid = newDetachedProcess(-1, KERNEL_PROCESS);

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
    Stacker<x86> kstack(((uint32_t) &proc.mainThreadInfo->storage) + THREAD_KSTACK_SIZE - 1);

    // EFLAGS: Clear V8086 , Clear Trap, Clear Nested Tasks.
    // Set Interrupt Enable Flag. IOPL = 3
    kstack << ((flags & (~0x24100)) | 0x3200);

    kstack << (uint32_t) KERNEL_CS_SEL;
    kstack << (uint32_t) entry;

    pushal_t regs;
    regs.esp = (uint32_t) kstack.getESP();
    regs.ebp = 0;
    regs.eax = 0;
    regs.ebx = regs.edx = 0;
    regs.ecx = (uint32_t) arg;     // fastcall passes first arg in ECX
    regs.edi = regs.esi = 0;

    kstack << regs;

    proc.mainThreadInfo->storage.pcb.esp0 = (target_esp0)kstack.getESP();

    return proc.mainThreadInfo;
}

void forceStartThread(thread_kinfo* thread)
{
    uint32_t flags;
    spin_lock_irqsave(&sched_lock, flags);

    if(!thread)
        return;
    cli();
    if(!cpu0_memmap.isStarted())
    {
        // Update fallback_txt_vmem for fallback putc() and clear()
        Maybe<uint32_t> vmemBase = virtOfPage0();
        char* vmemPage = NULL;
        if (vmemBase)
            vmemPage = (char*)(+vmemBase);
        else
        {
            printf("Fail to allocate virtual mem space for VMEM\n");
            asm volatile("1: hlt; jmp 1b;");
        }
        fallback_txt_vmem = vmemPage + PRE_INIT_VIDEO;

        cpu0_memmap.start();
    }
    cpu0_memmap.loadProcessMap(thread->getProcessDesc());

    // refresh TSS so that later interrupts use this new kstack
    tss.esp0 = (uint32_t)thread->storage.pcb.esp0;

    thread->getPCB()->runState = Running;

    // Add this thread to scheduler
    rrQueue->push_back(thread);

    spin_unlock(&sched_lock);

    asm volatile (
        "movl %0, %%esp         ;\n"
        "popal                  ;\n"
        "iretl                  ;\n"
        : : "rm" (thread->storage.pcb.esp0) : "cc");
    // This asm block changes everything but gcc should not worry about them.
}

void printDebug(thread_pcb* pcb, const char* str)
{
    uint32_t esp;
    asm volatile ("movl %%esp, %0" : "=rm" (esp));
    printf(str);
    printf("pid %d, currEsp=0x%x *iret=0x%x, 0x%x\n", pcb->to_process->getPid(), esp, *((uint32_t*)((uint32_t)pcb->esp0 + 32)), *((uint32_t*)((uint32_t)pcb->esp0 + 36)));
}

// Performs context switching
target_esp0 __attribute__((used)) schedDispatchExecution(target_esp0 currentESP)
{
    AutoSpinLock l(&sched_lock);

    if (num_nest_int() > 0)
        return NULL;
    if (wantToSwitchTo < 0)
    {
        /*
        if(getCurrentThreadInfo()->getProcessDesc()->getPid() != 1)
        printf("***switching to pid %d, map=0x%x submap=0x%x***n", getCurrentThreadInfo()->getProcessDesc()->getPid(), global_cr3val[0], userFirst4MBTable[184]);
        */

        getCurrentThreadInfo()->storage.pcb.esp0 = (target_esp0)((uint32_t) &getCurrentThreadInfo()->storage + THREAD_KSTACK_SIZE - 4);
        if(!getCurrentThreadInfo()->isKernel())
        {
            tss.esp0 = (uint32_t) getCurrentThreadInfo()->storage.pcb.esp0;
        }
        return NULL;
    }
    if (currentlyRunning == wantToSwitchTo)
        return NULL;

    ProcessDesc& desc = ProcessDesc::get(wantToSwitchTo);

    // Switch stack
    target_esp0 ans = desc.mainThreadInfo->storage.pcb.esp0;

    //printf("before switching-back map=0x%x ", global_cr3val[32]);

    // Switch Page Directory
    cpu0_memmap.loadProcessMap(&desc);

    // printDebug(desc.mainThreadInfo->getPCB(),"switching back to");

    currentlyRunning = wantToSwitchTo;
    // Reset dispatch decision state.
    wantToSwitchTo = -1;

    if(!desc.mainThreadInfo->isKernel())
    {
        // Reset TARGET thread's esp0 !!
        desc.mainThreadInfo->getPCB()->esp0 = (target_esp0)((uint32_t) &(desc.mainThreadInfo->storage) + THREAD_KSTACK_SIZE - 4);
    }

    // Save new kernel stack into TSS.
    //   so that later interrupts use this new kstack (AND we are at outmost interrupt, THUS we use ORIGINAL esp0)
    setTSS(desc.mainThreadInfo->storage.pcb);

    return ans;
}

void blockNotDuringInterrupt(thread_kinfo* thread)
{
    AutoSpinLock lock(&sched_lock);
    thread->getPCB()->runState = Waiting;
}

void block(thread_kinfo* thread)
{
    AutoSpinLock lock(&sched_lock);
    thread->getPCB()->runState = Waiting;
    makeDecisionNoLock();
}

void unblock(thread_kinfo* thread)
{
    AutoSpinLock lock(&sched_lock);
    thread->getPCB()->runState = Running;
}

void halt(thread_pcb& pcb, int32_t retval)
{
    AutoSpinLock l(&sched_lock);

    thread_kinfo* prevInfo = pcb.execParent;
    getRegs(prevInfo)->eax = retval;

    auto term = pcb.to_process->currTerm;
    // GET control of stdin.
    if(term)
    {
        term->tryDisableVidmap(&pcb);
        //term->setOwner(true, prevInfo->getProcessDesc()->getPid());
        term->setOwner(true, -1);
    }

    // Clean up process
    ProcessDesc::remove(pcb.to_process->getPid());
    // Remove process from run queue
    while ((*(rrQueue->front()))->getProcessDesc() != pcb.to_process)
    {
        thread_kinfo* tmp = *(rrQueue->front());
        rrQueue->pop_front();
        rrQueue->push_back(tmp);
    }
    rrQueue->pop_front();
    // Wake up parent
    prevInfo->getPCB()->runState = Running;

    // Make sure removed process is NOT scheduled after iret.
    makeDecisionNoLock();
}

}   // namespace scheduler

// ESP points to [pushal, iret, ...
void __attribute__((used)) schedBackupState(target_esp0 currentESP)
{
    AutoSpinLock l(&scheduler::sched_lock);

    // Should only be saved if this is the outmost interrupt.
    if (num_nest_int() > 0)
        return;

    /*
    if(getCurrentThreadInfo()->getProcessDesc()->getPid() != 1)
    printf("save pid %d to esp 0x%x, *=0x%x, 0x%x, 0x%x \n", getCurrentThreadInfo()->getProcessDesc()->getPid(), currentESP, *((uint32_t*)((uint32_t)currentESP + 0)), *((uint32_t*)((uint32_t)currentESP + 4)), *((uint32_t*)((uint32_t)currentESP + 8)));
    */

    // save current esp0 to current thread's pcb
    getCurrentThreadInfo()->storage.pcb.esp0 = currentESP;
}

target_esp0 __attribute__((used)) schedDispatchExecution(target_esp0 currentESP)
{
    return scheduler::schedDispatchExecution(currentESP);
}

// Note that it's not recommended to get current thread's regs,
//    and such actions will return NULL
pushal_t* getRegs(thread_kinfo* thread)
{
    if(thread == getCurrentThreadInfo())
        return NULL;
    if(!thread)
        return NULL;
    return (pushal_t*)thread->storage.pcb.esp0;
}

