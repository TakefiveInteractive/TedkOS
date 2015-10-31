
#ifndef _FILESYSTEM_WRAPPER_H_
#define _FILESYSTEM_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/multiboot.h>
#include <inc/lphashtable.h>
#include <inc/klibs/lib.h>
#include <inc/fs/filesystem.h>


#ifdef __cplusplus
namespace filesystem {
    void init_from_multiboot(multiboot_info *mbi);
    int32_t read_dentry(const uint8_t* fname, dentry_t* dentry);
    int32_t read_dentry(uint32_t index, dentry_t* dentry);
    int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
// C-side functions
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
#ifdef __cplusplus
}
#endif

#endif
