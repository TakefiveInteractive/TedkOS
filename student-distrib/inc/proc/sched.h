#ifndef _PROC_SCHED_H
#define _PROC_SCHED_H

#include <stddef.h>
#include <stdint.h>
#include <inc/x86/desc.h>
#include <inc/proc/tasks.h>

typedef void __attribute__((fastcall)) (*kthread_entry)(void*);

#ifdef __cplusplus
namespace scheduler {

    //      This will create a Process with:
    //          valid PCB ( NO esp (=> iretl, pushal, eax))
    //                    ( prev = next = NULL )
    //          valid PD  (thus FD)
    //          No executable in memory
    //          No PAGE TABLEs
    Pid newDetachedProcess(Pid parentPID, ProcessType processType);

    void attachThread(thread_kinfo* pcb, ThreadState newState);

    // Currently all switches are blocking:
    //      Parent process will wait for child to finish
    void prepareSwitchTo(int32_t pid);

    thread_kinfo* makeKThread(kthread_entry entry, void* arg);

    thread_kinfo* makeKThread(kthread_entry entry);

    // This is NOT a normal way to start thread!!! (only used to create process 0 )
    // Try to use prepareSwitchTo() inside an interrupt!!!
    void forceStartThread(thread_kinfo* thread);

    void enablePreemptiveScheduling();

    void makeDecision();

    // Cede current time slice
    void block(thread_kinfo* thread);
    void unblock(thread_kinfo* thread);

    // Common code to halt/squash a process
    void halt(thread_pcb& pcb, int32_t retval);

    // initialize global variables.
    void init();

}
#endif

#ifdef __cplusplus
extern "C" {
#endif
    void __attribute__((used)) schedBackupState(target_esp0 currentESP);
    target_esp0 schedDispatchExecution(target_esp0 currentESP);

    // Note that it's not recommended to get current thread's regs,
    //    and such actions will return NULL
    pushal_t* getRegs(thread_kinfo* thread);
#ifdef __cplusplus
}
#endif

#endif /* _PROC_SCHED_H */
