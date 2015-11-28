#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/fs/filesystem.h>
#include <inc/proc/tasks.h>

using namespace filesystem;
using namespace palloc;

namespace syscall { namespace fops {

int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    sti();
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    return theDispatcher->read(*processDesc->fileDescs[fd], buf, nbytes);
}

int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    return theDispatcher->write(*processDesc->fileDescs[fd], buf, nbytes);
}

int32_t open(const char *filename)
{
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
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

    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!processDesc->fileDescs.isValid(fd)) return -1;
    bool res = theDispatcher->close(*processDesc->fileDescs[fd]);
    if (!res) return -1;

    delete processDesc->fileDescs[fd];
    processDesc->fileDescs[fd] = nullptr;
    processDesc->fileDescs.recycle(fd);
    return 0;
}

} }

// Helps initializes the file descriptors of uniq_pid:
//  It assumes that the fd array is FILLED with NULL
int32_t init_fs_desc(ProcessDesc& proc)
{
    File *fd = new File;

    bool res = theDispatcher->open(*fd, "/dev/keyb");
    if (!res)
    {
        delete fd;
        return -1;
    }
    proc.fileDescs[0] = fd;

    fd = new File;
    res = theDispatcher->open(*fd, "/dev/term");
    if (!res)
    {
        delete fd;
        return -1;
    }
    proc.fileDescs[1] = fd;

    return 0;
}

