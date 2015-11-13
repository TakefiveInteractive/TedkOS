#include "exec.h"
#include <inc/error.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/klibs/kmalloc.h>
#include <inc/x86/paging.h>
#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/palloc.h>
#include <inc/x86/desc.h>
#include <inc/x86/stacker.h>

using namespace palloc;
using arch::Stacker;
using arch::CPUArchTypes::x86;

namespace syscall_exec
{

// Main entry to implementation of exec syscall
int32_t sysexec(const uint8_t* file)
{
    printf("Executing: %s\n", file);
    if(!file)
        return -1;

    uint32_t child_upid = do_exec(file);
    if(child_upid < 0)
        return -1;

    // Request context switch
    prepareSwitchTo(child_upid);
    return 0;
}

// Returns the uniq_pid of new process.
int32_t do_exec(const uint8_t* file)
{
    uint32_t flags;

    if(!is_kiss_executable(file))
        return -EINVAL;
    int32_t child_upid = newPausedProcess(getCurrentThreadInfo()->pcb.to_process->getUniqPid());

    if(child_upid < 0)
        return -1;          // Out of PIDs

    ProcessDesc& child = ProcessDesc::get(child_upid);

    // Allocate the page at 128MB virt. addr. for child
    uint16_t physIdx = physPages.allocPage(0);
    if(physIdx == 0xffff)
        return -1;          // Memory full.

    // !!! CHANGE THIS IF THIS IS A KERNEL THREAD !!!
    PhysAddr physAddr = PhysAddr(physIdx, PG_WRITABLE | PG_USER);

    if(!child.memmap.add(VirtAddr((void*)code_page_vaddr_base), physAddr))
        return -1;          // child virt addr space became weird.

    // Temporarily mount the address space to CURRENT context's virtual 128MB address.
    cli_and_save(flags);
    uint32_t backupDir = global_cr3val[code_page_vaddr_base >> 22];
    global_cr3val[code_page_vaddr_base >> 22] = physAddr.pde;
    RELOAD_CR3();

    // Load executable into memory
    void* entry_point = kiss_loader(file);

    // restore 128MB address back to CURRENT context's content
    global_cr3val[code_page_vaddr_base >> 22] = backupDir;
    RELOAD_CR3();
    restore_flags(flags);

    // Initialize stack and ESP
    // compatible with x86 32-bit iretl
    // always no error code on stack before iretl
    Stacker<x86> kstack((uint32_t)child.mainThreadInfo->kstack + THREAD_KSTACK_SIZE - 1);

    kstack << (uint32_t) USER_DS_SEL;
    kstack << (uint32_t) code_page_vaddr_base + (1<<22) - 8;

    // EFLAGS: Clear V8086 , Clear Trap, Clear Nested Tasks.
    // Set Interrupt Enable Flag. IOPL = 3
    kstack << ((flags & (~0x24100)) | 0x3200);

    kstack << (uint32_t) USER_CS_SEL;
    kstack << (uint32_t) entry_point;

    pushal_t regs;
    regs.esp = (uint32_t) kstack.getESP();
    regs.ebp = 0;
    regs.eax = -1;
    regs.ebx = regs.ecx = regs.edx = 0;
    regs.edi = regs.esi = 0;

    kstack << regs;

    child.mainThreadInfo->pcb.esp0 = (target_esp0)kstack.getESP();
    child.mainThreadInfo->pcb.isKernelThread = 0;

    // refresh TSS so that later interrupts use this new kstack
    tss.esp0 = (uint32_t)kstack.getESP();
    // ltr(KERNEL_TSS_SEL);     WILL CAUSE GENERAL PROTECTION ERROR
    return child_upid;
}

}
