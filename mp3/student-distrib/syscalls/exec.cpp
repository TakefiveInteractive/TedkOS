#include <inc/syscalls/exec.h>
#include <inc/syscalls/syscalls.h>
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
#include <inc/x86/idt_init.h>
#include <inc/drivers/kbterm.h>

using namespace palloc;
using namespace boost;
using arch::Stacker;
using arch::CPUArchTypes::x86;

namespace syscall { namespace exec {

// Main entry to implementation of exec syscall
int32_t sysexec(const char* file)
{
    if(!validUserPointer(file))
        return -1;

    int32_t child_upid = do_exec(file);
    if (child_upid < 0)
        return child_upid;

    // Request context switch
    scheduler::attachThread(ProcessDesc::get(child_upid).mainThreadInfo, Running);
    scheduler::makeDecision();
    return 0;
}

// return arg index in a filename, return -1 if no argument
int32_t get_arg_position(unique_ptr<char[]>& filename, uint32_t filename_len)
{
    uint32_t i = 0;
    int32_t arg_index = -1;
    for (; i < filename_len; i++)
    {
        if (filename[i] == ' ')
        {
            uint32_t j = i;
            for (; filename[j] == ' '; j++);
            if (filename[j] != '\0')
                arg_index = j;
            filename[i] = '\0';
            break;
        }
    }
    return arg_index;
}

void store_arg(unique_ptr<char[]>& filename, uint32_t filename_len, uint32_t arg_index, ProcessDesc& pd)
{
    uint32_t arg_len = filename_len - arg_index;
    pd.arg = new char[arg_len + 1];
    for (uint32_t i = 0; i < arg_len; i++)
        pd.arg[i] = filename[i + arg_index];
    pd.arg[arg_len] = '\0';
}

// Returns the pid of new process.
Pid do_exec(const char* arg0)
{
    return runWithoutNMI([arg0] () -> Pid
    {
        uint32_t filename_len = strlen(arg0);
        unique_ptr<char[]> file(new char[filename_len + 1]);

        // We need to copy filename into kernel because later we will switch page table
        for (size_t i = 0; i < filename_len; i++) file[i] = arg0[i];
        file[filename_len] = '\0';

        int32_t arg_index = get_arg_position(file, filename_len);

        if (!is_kiss_executable(file))
        {
            // "no such command"
            return -1;
        }
        Pid child_upid = scheduler::newDetachedProcess(
            getCurrentThreadInfo()->getProcessDesc()->getPid(),
            USER_PROCESS);

        if (child_upid < 0)
        {
            // "terminated by exception"
            return 256;          // Out of PIDs
        }

        ProcessDesc& child = ProcessDesc::get(child_upid);

        if (arg_index != -1)
        {
            store_arg(file, filename_len, arg_index, child);
        }

        // Allocate the page at 128MB virt. addr. for child
        auto physIdx = physPages.allocPage(false);
        if (!physIdx)
        {
            // "terminated by exception"
            return 256;          // Memory full.
        }

        // !!! CHANGE THIS IF THIS IS A KERNEL THREAD !!!
        PhysAddr physAddr = PhysAddr(+physIdx, PG_WRITABLE | PG_USER);

        if (!child.memmap.add(VirtAddr((void*)code_page_vaddr_base), physAddr))
        {
            // "terminated by exception"
            return 256;          // child virt addr space became weird.
        }

        child.heapStartingPageIdx = (code_page_vaddr_base >> 22) + 2;   // gap between heap and stack

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
        Stacker<x86> kstack((uint32_t) child.mainThreadInfo->storage.kstack + THREAD_KSTACK_SIZE - 1);

        kstack << (uint32_t) USER_DS_SEL;
        // Set up ESP for child process
        kstack << (uint32_t) code_page_vaddr_base + (1 << 22) - 8;

        // EFLAGS: Clear V8086 , Clear Trap, Clear Nested Tasks.
        // Set Interrupt Enable Flag. IOPL = 3
        kstack << (uint32_t)(((uint32_t) getFlagsRegister() & (~0x24100)) | 0x3200);

        kstack << (uint32_t) USER_CS_SEL;
        kstack << (uint32_t) entry_point;

        pushal_t regs;
        regs.esp = (uint32_t) kstack.getESP();
        regs.ebp = 0;
        regs.eax = -1;
        regs.ebx = regs.ecx = regs.edx = 0;
        regs.edi = regs.esi = 0;

        kstack << regs;

        child.mainThreadInfo->storage.pcb.esp0 = (target_esp0) kstack.getESP();

        // RELEASE control of stdin.
        if (getCurrentThreadInfo()->getProcessDesc()->currTerm)
            getCurrentThreadInfo()->getProcessDesc()->currTerm->setOwner(-1);

        return child_upid;
    });
}

} }
