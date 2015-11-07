#ifndef _TASKS_H
#define _TASKS_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
} process_pd;

// because we saves all register states in kernel stack,
//   here we do not repeat those states.
typedef struct _thread_pcb_t
{
    // Kernel stack state of current thread.
    uint32_t* esp0;
    process_pd* to_process;
    
    // Following is a simple list used by "scheduling"
    //    Simplest scheduling: process is paused and the next process
    //    to be executed is stored as current pcb->next.
    struct _thread_pcb_t *next, *prev;
} thread_pcb;

#define THREAD_KSTACK_SIZE          8192

typedef union
{
    thread_pcb pcb;
    uint8_t kstack[THREAD_KSTACK_SIZE];
} thread_kinfo;

#define THREAD_KINFO_SIZE           (sizeof(thread_kifo))

// CP3 Task switching policy:
//      PUSH means pause current process and run a new one
//      POP means terminate current process and return to previous one.

#ifdef __cplusplus
extern "C" {
#endif
    int8_t can_switch_task();
    void alert_sw_tsk_ban();
    void task_push(thread_kinfo* child);
    void task_pop();
    thread_kinfo* getCurrentThreadInfo();
#ifdef __cplusplus
}
#endif

#endif /* _TASKS_H */
