#ifndef _TERMINAL_FOPS_H
#define _TERMINAL_FOPS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
int32_t term_write(void* fdEntity, const uint8_t* buf, int32_t nbytes);
int32_t term_read(void* fdEntity, uint8_t* buf, int32_t nbytes);
int32_t term_open(void* fdEntity);
int32_t term_close(void* fdEntity);
#ifdef __cplusplus
}
#endif

#endif /* _TERMINAL_FOPS_H */
