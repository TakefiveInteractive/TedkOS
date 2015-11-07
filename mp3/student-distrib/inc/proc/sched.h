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
     target_esp0 schedDispatchDecision();
#ifdef __cplusplus
}
#endif


#endif /* _PROC_SCHED_H */
