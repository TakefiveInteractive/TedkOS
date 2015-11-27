#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/fs/filesystem.h>
#include <inc/proc/tasks.h>

using namespace filesystem;
using namespace palloc;

namespace syscall { namespace fops {

bool check_valid_fd(int32_t fd, ProcessDesc *processDesc)
{
    if (fd >= FD_ARRAY_LENGTH || fd < 0) return false;
    if (!processDesc->fileDescs[fd]) return false;
    return true;
}

int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    sti();
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!check_valid_fd(fd, processDesc)) return -1;
    return theDispatcher->read(*processDesc->fileDescs[fd], buf, nbytes);
}

int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!check_valid_fd(fd, processDesc)) return -1;
    return theDispatcher->write(*processDesc->fileDescs[fd], buf, nbytes);
}

int32_t open(const char *filename)
{
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    File *fd = new File;
    if (processDesc->numFilesInDescs >= FD_ARRAY_LENGTH)
        return -1;
    bool res = theDispatcher->open(*fd, filename);
    if (!res) {
        delete fd;
        return -1;
    }
    processDesc->fileDescs[processDesc->numFilesInDescs] = fd;
    return processDesc->numFilesInDescs++;
}

int32_t close(int32_t fd)
{
    // Disallow closing STDIN or STDOUT
    if (fd == 0 || fd == 1) return -1;

    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!check_valid_fd(fd, processDesc)) return -1;
    bool res = theDispatcher->close(*processDesc->fileDescs[fd]);
    if (!res) return -1;
    if (processDesc->fileDescs[fd])
    {
        delete processDesc->fileDescs[fd];
        processDesc->fileDescs[fd] = nullptr;
    }
    return 0;
}

int32_t fstat(int32_t fd, stat *st)
{
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!check_valid_fd(fd, processDesc)) return -1;
    return theDispatcher->fstat(*processDesc->fileDescs[fd], st);
}

int32_t lseek(int32_t fd, int32_t offset, int32_t whence)
{
    auto processDesc = getCurrentThreadInfo()->pcb.to_process;
    if (!check_valid_fd(fd, processDesc)) return -1;
    return theDispatcher->lseek(*processDesc->fileDescs[fd], offset, whence);
}

} }

// Helps initializes the file descriptors of uniq_pid:
//  It assumes that the fd array in the process is
//  completely not intialized (including numFilesInDescs)
int32_t init_fs_desc(ProcessDesc& proc)
{
    proc.numFilesInDescs = 2;
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

