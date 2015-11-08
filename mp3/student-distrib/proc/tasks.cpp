#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>

ProcessDesc::ProcessDesc()
{
    memset(fileDescs, 0, sizeof(threadPCBs));
    mainThreadInfo = new thread_kinfo;

    mainThreadInfo->pcb.esp0 = NULL;
    mainThreadInfo->pcb.to_process = this;
    mainThreadInfo->pcb.next = NULL;
    mainThreadInfo->pcb.prev = NULL;

    // -1 is a good invalid value for physical addr.
    pageDir = (uint32_t*)-1;
}

void ProcessDesc::init()
{
    if(!all_processes)
        all_processes = new ProcessDesc[MAX_NUM_PROCESS];
}

ProcessDesc* ProcessDesc::all()
{
    init();
    return all_processes;
}

ProcessDesc& ProcessDesc::get(size_t uniq_pid)
{
    init();
    return all_processes[uniq_pid];
}

int32_t ProcessDesc::getUniqPid()
{
    return this - all_processes;
}

size_t ProcessDesc::newProcess()
{
    init();
    return all_processes[nextNewProcess++];
}

