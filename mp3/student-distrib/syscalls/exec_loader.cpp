#include "exec.h"
#include <inc/error.h>
#include <inc/fs/kiss_wrapper.h>

namespace syscall_exec {

//   Load "file", an executable, to "program_image_addr"
//   You can assume that CR3 already points the page table of this process.
// Return 0 if success. Return -ENOMEM if file is too large (not possible, though)
void* kiss_loader(const uint8_t* file)
{
    dentry_t dentry;
    int32_t res = read_dentry_by_name(file, &dentry);
    if (res == -1) return NULL;

    const uint32_t four_mb = 0x400000;
    uint8_t* curr = (uint8_t*)program_image_addr;
    uint8_t test_over_flow;
    uint32_t first_len = read_data(dentry.inode, 0, curr, four_mb);
    if (first_len == four_mb)
        if (read_data(dentry.inode, four_mb, &test_over_flow, 1) > 0)
            return NULL;

    return *(uint32_t**)(program_image_addr + 24);
}

}
