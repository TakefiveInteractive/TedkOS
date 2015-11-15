#include <inc/x86/desc.h>

.global iret_sched_policy
.text

# KStack must look like: [pushal, iret info]
# Must use 'jmp' to "call" this function
iret_sched_policy:

    pushl %esp                         # current stack frame is important, should be saved by schedDispatchDecision
    call schedDispatchDecision         # return NULL or *** the esp0 to switch to ***
    addl $4, %esp

    testl %eax, %eax        
    jz iret_sched_policy_noswitch      # Jump if no kernel stack switching.

    movl %eax, %esp                  # Switch the kernel stack!

iret_sched_policy_noswitch:                  
    call isCurrThreadKernel   
    testl %eax, %eax        
    jnz iret_sched_policy_kthread      # Jump if new thread is kernel thread
    movl $USER_DS_SEL, %ecx            
    movw %cx, %ds           
    movw %cx, %es           
    movw %cx, %fs           
    movw %cx, %gs           
iret_sched_policy_kthread:
    popal 
    iretl  

