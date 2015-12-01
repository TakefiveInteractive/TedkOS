#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/syscalls/syscalls.h>
#include <inc/fs/filesystem.h>
#include <inc/proc/tasks.h>

using namespace filesystem;
using namespace palloc;

namespace syscall { namespace fops {

int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    if(!validUserPointer(buf))
        return -1;
    sti();
    auto processDesc = getCurrentThreadInfo()->getProcessDesc();
    if (!ensureFdInitialized(processDesc)) return -1;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    return theDispatcher->read(*processDesc->fileDescs[fd], buf, nbytes);
}

int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    if(!validUserPointer(buf))
        return -1;
    auto processDesc = getCurrentThreadInfo()->getProcessDesc();
    if (!ensureFdInitialized(processDesc)) return -1;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    return theDispatcher->write(*processDesc->fileDescs[fd], buf, nbytes);
}

int32_t open(const char *filename)
{
    if(!validUserPointer(filename))
        return -1;
    auto processDesc = getCurrentThreadInfo()->getProcessDesc();
    if (!ensureFdInitialized(processDesc)) return -1;
    File *fd = new File;
    auto fdSlotMaybe = processDesc->fileDescs.alloc();
    if(!fdSlotMaybe)
        return -1;
    bool res = theDispatcher->open(*fd, filename);
    if (!res) {
        delete fd;
        return -1;
    }
    processDesc->fileDescs[+fdSlotMaybe] = fd;
    return +fdSlotMaybe;
}

int32_t close(int32_t fd)
{
    // Disallow closing STDIN or STDOUT
    if (fd == 0 || fd == 1) return -1;

    auto processDesc = getCurrentThreadInfo()->getProcessDesc();
    if (!ensureFdInitialized(processDesc)) return -1;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    bool res = theDispatcher->close(*processDesc->fileDescs[fd]);
    if (!res) return -1;

    delete processDesc->fileDescs[fd];
    processDesc->fileDescs[fd] = nullptr;
    processDesc->fileDescs.recycle(fd);
    return 0;
}

int32_t fstat(int32_t fd, stat *st)
{
    auto processDesc = getCurrentThreadInfo()->getProcessDesc();
    if (!ensureFdInitialized(processDesc)) return -1;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    return theDispatcher->fstat(*processDesc->fileDescs[fd], st);
}

int32_t lseek(int32_t fd, int32_t offset, int32_t whence)
{
    auto processDesc = getCurrentThreadInfo()->getProcessDesc();
    if (!ensureFdInitialized(processDesc)) return -1;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    return theDispatcher->lseek(*processDesc->fileDescs[fd], offset, whence);
}

// Helps initializes the file descriptors of uniq_pid:
bool ensureFdInitialized(ProcessDesc* proc)
{
    if (proc->fdInitialized) return true;

    File *fd = new File;

    bool res = theDispatcher->open(*fd, "/dev/keyb");
    if (!res)
    {
        delete fd;
        return false;
    }
    proc->fileDescs[0] = fd;

    fd = new File;
    res = theDispatcher->open(*fd, "/dev/term");
    if (!res)
    {
        delete fd;
        return false;
    }
    proc->fileDescs[1] = fd;

    proc->fdInitialized = true;
    return true;
}

} }

