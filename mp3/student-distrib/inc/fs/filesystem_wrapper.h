#ifndef _FILESYSTEM_WRAPPER_H_
#define _FILESYSTEM_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// C-side functions
int32_t fs_read(int32_t fd, void *buf, int32_t nbytes);
int32_t fs_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t fs_open(const char *filename);
int32_t fs_close(int32_t fd);
#ifdef __cplusplus
}
#endif

#endif
