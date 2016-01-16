#include <inc/proc/tasks.h>

.global getCurrentThreadInfo
.text

getCurrentThreadInfo:
    movl %esp, %eax
    andl $THREAD_ESP_TO_PCB, %eax
    ret
