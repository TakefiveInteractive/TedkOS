#ifndef _FILESYSTEM_WRAPPER_H_
#define _FILESYSTEM_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/proc/tasks.h>

namespace syscall { namespace fops {

int32_t read(int32_t fd, void *buf, int32_t nbytes);
int32_t write(int32_t fd, const void *buf, int32_t nbytes);
int32_t open(const char *filename);
int32_t close(int32_t fd);
int32_t fstat(int32_t fd, stat *st);
int32_t lseek(int32_t fd, int32_t offset, int32_t whence);

} }

// Helps initializes the file descriptors of uniq_pid:
//  It assumes that the fd array in the process is
//  completely not intialized (including numFilesInDescs)
int32_t init_fs_desc(ProcessDesc& proc);

#endif
