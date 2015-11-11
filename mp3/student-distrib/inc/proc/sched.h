#ifndef _PROC_SCHED_H
#define _PROC_SCHED_H

#include <stddef.h>
#include <stdint.h>

// This type stores the esp of the kernel stack of the thread to switch to.
//      Use NULL if not going to switch.
typedef void* target_esp0;

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
    target_esp0 schedDispatchDecision();
#ifdef __cplusplus
}
#endif


#endif /* _PROC_SCHED_H */
