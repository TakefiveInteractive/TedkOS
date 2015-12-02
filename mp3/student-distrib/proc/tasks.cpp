#include <inc/proc/tasks.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/new.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/fs/filesystem.h>
#include <inc/klibs/memory.h>
#include <inc/syscalls/exec.h>

using memory::operator "" _MB;

size_t ProcessDesc::nextNewProcess = 0;
ProcessDesc *static_all_processes[MAX_NUM_PROCESS] = { };
ProcessDesc **ProcessDesc::all_processes = static_all_processes;

ProcessDesc::ProcessDesc(int32_t _pid, ProcessType processType)
    : fileDescs(), fdInitialized(false), heapPhysicalPages(),
    heapStartingPageIdx(0), heapSize(0), numHeapPages(0),
    fileName(nullptr), arg(nullptr)
{
    mainThreadInfo = new thread_kinfo(this, processType);

    pid = _pid;
}

ProcessDesc::~ProcessDesc()
{
    delete mainThreadInfo;
    // Free all heap memory
    while (heapPhysicalPages.empty() == false)
    {
        auto idx = heapPhysicalPages.pop();
        physPages.freePage(idx);
    }
    // Close all files
    for (size_t i = 0; i < FD_ARRAY_LENGTH; i++)
    {
        if (fileDescs[i])
        {
            filesystem::theDispatcher->close(*fileDescs[i]);
            delete fileDescs[i];
            fileDescs[i] = nullptr;
        }
    }
    // Free program memory
    auto idx = cpu0_memmap.translate(VirtAddr((void *) syscall::exec::code_page_vaddr_base));
    physPages.freePage(idx);
}

void* ProcessDesc::sbrk(int32_t delta)
{
    auto oldHeapSize = heapSize;
    // Contracting heap beyond its starting address?
    if (delta < 0 && heapSize < (size_t)(-delta)) return NULL;
    heapSize += delta;
    while (heapSize > numHeapPages * 4_MB)
    {
        // append a page
        auto physAddr = physPages.allocPage(true);
        if (!physAddr) return NULL;
        if (!memmap.add(
            VirtAddr((uint8_t *)((heapStartingPageIdx + numHeapPages) * 4_MB)),
            PhysAddr(+physAddr, PG_WRITABLE | PG_USER))) return NULL;
        heapPhysicalPages.push(+physAddr);
        numHeapPages++;
    }
    while (heapSize + 4_MB <= numHeapPages * 4_MB)
    {
        // free top most page
        numHeapPages--;
        auto virtAddr = (VirtAddr((uint8_t *)((heapStartingPageIdx + numHeapPages) * 4_MB)));
        auto physAddr = cpu0_memmap.translate(virtAddr);
        auto ourPhysAddr = PhysAddr(heapPhysicalPages.pop(), PG_WRITABLE | PG_USER);
        // physical addresses don't match up
        if (physAddr != ourPhysAddr) return NULL;
        physPages.freePage(physAddr);
        // Delete it from paging system
        memmap.remove(virtAddr, physAddr);
    }
    if (!cpu0_memmap.loadProcessMap(this)) return NULL;
    return reinterpret_cast<void *>(heapStartingPageIdx * 4_MB + oldHeapSize);
}

ProcessDesc** ProcessDesc::all()
{
    return all_processes;
}

ProcessDesc& ProcessDesc::get(size_t pid)
{
    return *all_processes[pid];
}

void ProcessDesc::remove(size_t pid)
{
    if (all_processes[pid])
    {
        delete all_processes[pid];
        all_processes[pid] = nullptr;
    }
}

int32_t ProcessDesc::getPid() const
{
    return pid;
}

ProcessDesc& ProcessDesc::newProcess(int32_t parentPID, ProcessType processType)
{
    const int32_t pid = nextNewProcess;
    ProcessDesc *p = new ProcessDesc(pid, processType);

    thread_kinfo* parentInfo = NULL;
    if (parentPID >= 0)
        parentInfo = ProcessDesc::get(parentPID).mainThreadInfo;
    p->mainThreadInfo->storage.pcb.prev = parentInfo;
    if (parentInfo)
        parentInfo->storage.pcb.next = p->mainThreadInfo;

    if(!all_processes[pid])
        all_processes[pid] = p;

    nextNewProcess++;
    return *p;
}

FileDescArr::FileDescArr()
{
    memset(content, 0, sizeof(content));
    size_t i = 0;
    for(; i < FD_FIXED_PART; i++)
        isFDfree.clear(i);
    for(; i < FD_ARRAY_LENGTH; i++)
    {
        isFDfree.set(i);
        freeFDs.push(i);
    }
}

filesystem::File*& FileDescArr::operator[] (const size_t i)
{
    return content[i];
}

Maybe<size_t> FileDescArr::alloc()
{
    if(freeFDs.empty())
        return Maybe<size_t>();
    else
    {
        size_t ans = freeFDs.pop();
        isFDfree.clear(ans);
        return ans;
    }
}

bool FileDescArr::isValid(size_t i)
{
    if(i < 0 || i >= FD_ARRAY_LENGTH)
        return false;
    else if(isFDfree.test(i))
        return false;
    else if(content[i] == NULL)
        return false;
    else return true;
}

void FileDescArr::recycle(size_t i)
{
    if(isFDfree.test(i))
        return;
    isFDfree.set(i);
    freeFDs.push(i);
}

uint8_t __attribute__((used)) isCurrThreadKernel()
{
    return getCurrentThreadInfo()->isKernel();
}


