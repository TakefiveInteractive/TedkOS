#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/new.h>

size_t ProcessDesc::nextNewProcess = 0;
ProcessDesc **ProcessDesc::all_processes = NULL;

ProcessDesc::ProcessDesc(int32_t _upid)
{
    memset(fileDescs, 0, sizeof(fileDescs));
    mainThreadInfo = new thread_kinfo;

    mainThreadInfo->pcb.esp0 = NULL;
    mainThreadInfo->pcb.to_process = this;
    mainThreadInfo->pcb.next = NULL;
    mainThreadInfo->pcb.prev = NULL;

    upid = _upid;
}

void ProcessDesc::init()
{
    if(!all_processes)
    {
        all_processes = new ProcessDesc*[MAX_NUM_PROCESS];
        memset(all_processes, 0, sizeof(all_processes));
    }
}

ProcessDesc** ProcessDesc::all()
{
    init();
    return all_processes;
}

ProcessDesc& ProcessDesc::get(size_t uniq_pid)
{
    init();
    if(!all_processes[uniq_pid])
        all_processes[uniq_pid] = new ProcessDesc(uniq_pid);
    return *all_processes[uniq_pid];
}

int32_t ProcessDesc::getUniqPid()
{
    return upid;
}

size_t ProcessDesc::newProcess()
{
    init();
    return nextNewProcess++;
}

