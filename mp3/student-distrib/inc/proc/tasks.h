#ifndef _TASKS_H
#define _TASKS_H

/* This two MUST BE EQUAL and POWER OF TWO !!! */
#define THREAD_KSTACK_SIZE          8192
#define THREAD_KINFO_SIZE           THREAD_KSTACK_SIZE

#define THREAD_ESP_TO_PCB           ~(THREAD_KINFO_SIZE - 1)

#ifndef ASM

#include <stddef.h>
#include <stdint.h>
#include <inc/proc/sched.h>
#include <inc/fs/filesystem.h>
#include <inc/klibs/palloc.h>

using namespace palloc;

#define MAX_NUM_PROCESS             256

#define FD_ARRAY_LENGTH             128
#define MAX_NUM_THREADS             128

class ProcessDesc
{
private:
    static size_t nextNewProcess;
    static ProcessDesc** all_processes;
    ProcessDesc(int32_t _upid);
    int32_t upid;
public:
    static ProcessDesc** all();
    static ProcessDesc& get(size_t uniq_pid);
    static size_t newProcess();
    int32_t getUniqPid();
    filesystem::File *fileDescs[FD_ARRAY_LENGTH];
    int32_t numFilesInDescs;
    // Currently no multithread
    union _thread_kinfo * mainThreadInfo;
    TinyMemMap memmap;

    // For get args
    char *fileName;
    char *arg;
};


// because we saves all register states in kernel stack,
//   here we do not repeat those states.
typedef struct _thread_pcb_t
{
    // Kernel stack state of current thread.
    volatile target_esp0 esp0;
    ProcessDesc* to_process;

    // If this is a kernel thread, non-zero. Otherwise zero.
    uint8_t isKernelThread;

    // Following is a simple list used by "scheduling"
    //    Simplest scheduling: process is paused and the next process
    //    to be executed is stored as current pcb->next.
    union _thread_kinfo *next, *prev;
} thread_pcb;

typedef union _thread_kinfo
{
    thread_pcb pcb;
    uint8_t kstack[THREAD_KSTACK_SIZE];
} thread_kinfo;

//!!!! thread_kinfo must be aligned in memory !!!!


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
