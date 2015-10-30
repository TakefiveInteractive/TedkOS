#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/multiboot.h>

struct dentry_t {
    char filename[33] = {};
    uint32_t filetype;
    uint32_t inode_idx;
}

struct inode_t {
    uint32_t size;
    uint32_t datablocks[4096 / sizeof(uint32_t) - 1];
}

#ifdef __cplusplus
namespace filesystem {

    class KissFS {
    private:
        LinearProbingHashTable<133, char *, uint32_t, KissFS::HashFunc> inodeIndexOfFilename;

        static const uint32_t MaxFilenameLength = 32;

        static uint32_t HashFunc(char *s)
        {
            uint32_t hash = 0;
            uint32_t used = 0;
            while (*s && used < MaxFilenameLength)
            {
                hash = hash * 101 + *s++;
                used++;
            }
            return hash;
        }
    };

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
