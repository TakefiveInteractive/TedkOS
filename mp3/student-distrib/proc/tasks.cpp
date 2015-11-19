#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/new.h>
#include <inc/syscalls/filesystem_wrapper.h>

size_t ProcessDesc::nextNewProcess = 0;
ProcessDesc *static_all_processes[MAX_NUM_PROCESS] = { };
ProcessDesc **ProcessDesc::all_processes = static_all_processes;

ProcessDesc::ProcessDesc(int32_t _upid) : fileDescs(), numFilesInDescs(0)
{
    mainThreadInfo = new thread_kinfo;

    mainThreadInfo->pcb.esp0 = NULL;
    mainThreadInfo->pcb.to_process = this;
    mainThreadInfo->pcb.next = NULL;
    mainThreadInfo->pcb.prev = NULL;

    upid = _upid;

    init_fs_desc(*this);
}

ProcessDesc::~ProcessDesc()
{
    delete mainThreadInfo;
}

ProcessDesc** ProcessDesc::all()
{
    return all_processes;
}

ProcessDesc& ProcessDesc::get(size_t uniq_pid)
{
    if(!all_processes[uniq_pid])
        all_processes[uniq_pid] = new ProcessDesc(uniq_pid);
    return *all_processes[uniq_pid];
}

void ProcessDesc::remove(size_t uniq_pid)
{
    if (all_processes[uniq_pid])
    {
        delete all_processes[uniq_pid];
        all_processes[uniq_pid] = nullptr;
    }
}

int32_t ProcessDesc::getUniqPid()
{
    return upid;
}

size_t ProcessDesc::newProcess()
{
    return nextNewProcess++;
}

uint8_t __attribute__((used)) isCurrThreadKernel()
{
    return getCurrentThreadInfo()->pcb.isKernelThread;
}


