#ifndef _MBI_INFO_H_
#define _MBI_INFO_H_

#include <inc/multiboot.h>

#ifdef __cplusplus
extern "C" {
#endif

void mbi_info (unsigned long magic, unsigned long addr);

#ifdef __cplusplus
}
#endif

extern multiboot_info_t *MultiBootInfoAddress;

#endif

