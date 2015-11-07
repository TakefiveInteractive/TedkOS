#include <inc/tasks.h>

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

thread_kinfo* getCurrentThreadInfo()
{
}
