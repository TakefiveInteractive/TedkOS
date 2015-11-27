#ifndef _PROC_SCHED_H
#define _PROC_SCHED_H

#include <stddef.h>
#include <stdint.h>
#include <inc/x86/desc.h>

// This type stores the esp of the kernel stack of the thread to switch to.
//      Use NULL if not going to switch.
typedef void* target_esp0;

typedef void (*kthread_entry)(void*);

#ifdef __cplusplus
extern "C" {
#endif
    //      This will create a Process with:
    //          valid PCB ( NO esp (=> iretl, pushal, eax))
    //                    ( prev = next = NULL )
    //          valid PD  (thus FD)
    //          No executable in memory
    //          No PAGE TABLEs
    int32_t newPausedProcess(int32_t parentPID);
    // Currently all switches are blocking:
    //      Parent process will wait for child to finish
    void prepareSwitchTo(int32_t pid);

    target_esp0 schedDispatchExecution(target_esp0 currentESP);

    // Note that it's not recommended to get current thread's regs,
    //    and such actions will return NULL
    pushal_t* getRegs(union _thread_kinfo* thread);

    union _thread_kinfo* makeKThread(kthread_entry entry, void* arg);

    // This is NOT a normal way to start thread!!! (only used to create process 0 )
    // Try to use prepareSwitchTo() inside an interrupt!!!
    void forceStartThread(union _thread_kinfo* thread);

#ifdef __cplusplus
}
#endif

void enablePreemptiveScheduling();
void schedMakeDecision();

#endif /* _PROC_SCHED_H */
