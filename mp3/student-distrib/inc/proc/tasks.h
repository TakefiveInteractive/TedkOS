#ifndef _TASKS_H
#define _TASKS_H

/* This two MUST BE EQUAL and POWER OF TWO !!! */
#define THREAD_KSTACK_SIZE          8192
#define THREAD_KINFO_SIZE           THREAD_KSTACK_SIZE

#define THREAD_ESP_TO_PCB           ~(THREAD_KINFO_SIZE - 1)

#ifndef ASM

#include <stddef.h>
#include <stdint.h>
#include <inc/fs/filesystem.h>
#include <inc/klibs/palloc.h>
#include <inc/drivers/kbterm.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/bitset.h>
#include <inc/klibs/maybe.h>

using namespace palloc;
using namespace util;

#define MAX_NUM_PROCESS             256
#define MAX_NUM_THREADS             128

class ProcessDesc;
struct thread_kinfo;
typedef int32_t Pid;
typedef int32_t Tid;

// This type stores the esp of the kernel stack of the thread to switch to.
//      Use NULL if not going to switch.
typedef void* target_esp0;

enum ProcessType
{
    KERNEL_PROCESS = 1,
    USER_PROCESS
};

enum ThreadState
{
    New = 1,
    Running,
    Waiting,            // Waiting -- not able to be scheduled
    Terminated
};

// because we saves all register states in kernel stack,
//   here we do not repeat those states.
struct thread_pcb
{
    // Kernel stack state of current thread.
    volatile target_esp0 esp0;
    ProcessDesc* to_process;

    ProcessType type;

    ThreadState runState;

    // Following is a simple list used by "scheduling"
    //    EXEC     scheduling: process is paused and the next process
    //    to be executed is stored as current pcb->next.
    thread_kinfo *execChild, *execParent;
};

//!!!! thread_kinfo must be aligned in memory !!!!
struct __attribute__ ((__packed__)) thread_kinfo
{
    union {
        uint8_t kstack[THREAD_KSTACK_SIZE];
        thread_pcb pcb;
    } storage;

    const uint8_t* getStackBoundary () const { return storage.kstack; }

    uint8_t* getStackBoundary () { return storage.kstack; }

    thread_pcb* getPCB() { return &storage.pcb; }

    ProcessDesc* getProcessDesc() { return storage.pcb.to_process; }

    bool isKernel() { return storage.pcb.type == KERNEL_PROCESS; }

    thread_kinfo(ProcessDesc *parent, ProcessType processType)
    {
        storage.pcb.esp0 = NULL;
        storage.pcb.to_process = parent;
        storage.pcb.execChild = NULL;
        storage.pcb.execParent = NULL;
        storage.pcb.type = processType;
        storage.pcb.runState = New;
    }

    void copy(const struct thread_kinfo& other)
    {
        memcpy(this, &other, sizeof(thread_kinfo));
        storage.pcb.esp0 = ((target_esp0)((uint32_t)other.storage.pcb.esp0 - (uint32_t)other.getStackBoundary() + (uint32_t)getStackBoundary()));
    }

    // Completely copy (not changing anything except actual stack address)
    thread_kinfo(const struct thread_kinfo& other)
    {
        memcpy(this, &other, sizeof(thread_kinfo));
        storage.pcb.esp0 = ((target_esp0)((uint32_t)other.storage.pcb.esp0 - (uint32_t)other.getStackBoundary() + (uint32_t)getStackBoundary()));
    }
};

static_assert(sizeof(thread_kinfo) == THREAD_KSTACK_SIZE, "thread_kinfo size mismatch");

#define FD_ARRAY_LENGTH             128
#define FD_FIXED_PART               2
#define FD_CYCLABLE_PART            (FD_ARRAY_LENGTH - FD_FIXED_PART)
// There are 2 FDs that are fixed in one process.

// An array to manage reusable File Descs
class FileDescArr
{
    filesystem::File *content[FD_ARRAY_LENGTH];
    Stack<size_t, FD_ARRAY_LENGTH> freeFDs;
    BitSet<FD_ARRAY_LENGTH> isFDfree;
public:
    FileDescArr();
    filesystem::File*& operator[] (const size_t i);

    Maybe<size_t> alloc();
    void recycle(size_t i);

    bool isValid(size_t i);
};

class ProcessDesc
{
private:
    static size_t nextNewProcess;
    static ProcessDesc** all_processes;
    ProcessDesc(int32_t _upid, ProcessType processType);
    Pid pid;

public:
    static ProcessDesc** all();
    static bool has(Pid pid);
    static ProcessDesc& get(Pid pid);
    static void remove(Pid pid);
    static ProcessDesc& newProcess(int32_t _upid, ProcessType processType);

    ~ProcessDesc();
    Pid getPid() const;

    FileDescArr fileDescs;
    bool fdInitialized;

    // Currently no multi-thread support
    thread_kinfo *mainThreadInfo;
    TinyMemMap memmap;

    util::Stack<uint16_t, 1024> heapPhysicalPages;
    uint16_t heapStartingPageIdx;
    size_t heapSize;
    uint16_t numHeapPages;

    void *sbrk(int32_t delta);

    // For get args
    char *fileName;
    char *arg;

    // For terminal and keyboard driver to find out current terminal.
    // currTerm is a process state, regardless of whether KeyB/Term is opened,
    //     so it's not saved in fileDescs
    Term::Term* currTerm;
};

#ifdef __cplusplus
extern "C" {
#endif
    thread_kinfo* getCurrentThreadInfo();
    uint8_t isCurrThreadKernel();
#ifdef __cplusplus
}
#endif

#endif

#endif /* _TASKS_H */
