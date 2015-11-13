#include <inc/fs/filesystem_wrapper.h>
#include <inc/fs/filesystem.h>

using namespace filesystem;

int32_t fs_read(int32_t fd, void *buf, int32_t nbytes)
{
    return theDispatcher->read(fd, buf, nbytes);
}

int32_t fs_write(int32_t fd, const void *buf, int32_t nbytes)
{
    return theDispatcher->write(fd, buf, nbytes);
}

int32_t fs_open(const char *filename)
{
    return theDispatcher->open(filename);
}

int32_t fs_close(int32_t fd)
{
    return theDispatcher->close(fd);
}

