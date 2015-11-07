#include <inc/tasks.h>
#include <inc/klibs/lib.h>

// If child == NULL, it will crash
// If curr already has a child (not possible), will crash.
void task_push(thread_kinfo* child)
{
    thread_kinfo* curr = getCurrentThreadInfo();
    thread_pcb* currpcb = &curr->pcb;
    thread_pcb* childpcb = &child->pcb;
    currpcb->next = childpcb;
    childpcb->prev = currpcb;
}

void task_pop()
{
    thread_kinfo* curr = getCurrentThreadInfo();
}

int8_t can_switch_task()
{
    return 0;
}

void alert_sw_tsk_ban()
{
    printf("ERROR: task switch is not allowed but forced to complete\n");
}

thread_kinfo* getCurrentThreadInfo()
{
}
