#include "exec.h"
#include <inc/error.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/klibs/kmalloc.h>
#include <inc/x86/paging.h>
#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/palloc.h>
#include <inc/klibs/lib.h>
#include <inc/x86/desc.h>
#include <inc/x86/stacker.h>
#include <inc/fops_kb.h>

using namespace palloc;
using namespace boost;
using arch::Stacker;
using arch::CPUArchTypes::x86;

namespace syscall_exec
{

// Main entry to implementation of exec syscall
int32_t sysexec(const char* file)
{
    printf("Executing: %s\n", file);
    if(!file)
        return -1;

    int32_t child_upid = do_exec(file);
    if(child_upid < 0)
        return child_upid;

    // Request context switch
    prepareSwitchTo(child_upid);
    return 0;
}

// Returns the uniq_pid of new process.
int32_t do_exec(const char* arg0)
{
    uint32_t flags;
    cli_and_save(flags);

    uint32_t filename_len = strlen(arg0);
    unique_ptr<char[]> file(new char[filename_len + 1]);

    // We need to copy filename into kernel because later we will switch page table
    for (size_t i = 0; i < filename_len; i++) file[i] = arg0[i];
    file[filename_len] = '\0';

    if(!is_kiss_executable(file))
    {
        restore_flags(flags);
        // "no such command"
        return -1;
    }
    int32_t child_upid = newPausedProcess(getCurrentThreadInfo()->pcb.to_process->getUniqPid());

    if(child_upid < 0)
    {
        restore_flags(flags);
        // "terminated by exception"
        return 256;          // Out of PIDs
    }

    ProcessDesc& child = ProcessDesc::get(child_upid);

    // Allocate the page at 128MB virt. addr. for child
    uint16_t physIdx = physPages.allocPage(0);
    if(physIdx == 0xffff)
    {
        restore_flags(flags);
        // "terminated by exception"
        return 256;          // Memory full.
    }

    // !!! CHANGE THIS IF THIS IS A KERNEL THREAD !!!
    PhysAddr physAddr = PhysAddr(physIdx, PG_WRITABLE | PG_USER);

    if(!child.memmap.add(VirtAddr((void*)code_page_vaddr_base), physAddr))
    {
        restore_flags(flags);
        // "terminated by exception"
        return 256;          // child virt addr space became weird.
    }

    // Temporarily mount the address space to CURRENT context's virtual 128MB address.
    uint32_t backupDir = global_cr3val[code_page_vaddr_base >> 22];
    global_cr3val[code_page_vaddr_base >> 22] = physAddr.pde;
    RELOAD_CR3();

    // Load executable into memory
    void* entry_point = kiss_loader(file);

    // restore 128MB address back to CURRENT context's content
    global_cr3val[code_page_vaddr_base >> 22] = backupDir;
    RELOAD_CR3();

    // Initialize stack and ESP
    // compatible with x86 32-bit iretl
    // always no error code on stack before iretl
    Stacker<x86> kstack((uint32_t)child.mainThreadInfo->kstack + THREAD_KSTACK_SIZE - 1);

    kstack << (uint32_t) USER_DS_SEL;
    kstack << (uint32_t) code_page_vaddr_base + (1<<22) - 8;

    // EFLAGS: Clear V8086 , Clear Trap, Clear Nested Tasks.
    // Set Interrupt Enable Flag. IOPL = 3
    kstack << (uint32_t)(((uint32_t)flags & (~0x24100)) | 0x3200);

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

    // RELEASE control of stdin.
    kb_close(NULL);

    restore_flags(flags);
    return child_upid;
}

}
