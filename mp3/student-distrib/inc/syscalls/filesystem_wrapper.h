#ifndef _FILESYSTEM_WRAPPER_H_
#define _FILESYSTEM_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/proc/tasks.h>

int32_t fs_read(int32_t fd, void *buf, int32_t nbytes);
int32_t fs_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t fs_open(const char *filename);
int32_t fs_close(int32_t fd);

// Helps initializes the file descriptors of uniq_pid:
//  It assumes that the fd array in the process is 
//  completely not intialized (including numFilesInDescs)
int32_t init_fs_desc(ProcessDesc& proc);

#endif
