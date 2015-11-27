#include <inc/syscalls/exec.h>
#include <inc/error.h>
#include <inc/fs/filesystem.h>

using namespace filesystem;

namespace syscall { namespace exec {

/* Load "file", an executable, to "program_image_addr"
 * You can assume that CR3 already points the page table of this process.
 * Return 0 if success. Return -ENOMEM if file is too large (not possible, though)
 */
void* kiss_loader(const boost::unique_ptr<char[]> &filename)
{
    File file;
    if (!theDispatcher->open(file, filename.get())) return NULL;

    const uint32_t four_mb = 0x400000;
    uint8_t* curr = (uint8_t*)program_image_addr;
    uint8_t test_over_flow;
    uint32_t first_len = theDispatcher->read(file, curr, four_mb);

    bool fail = false;
    if (first_len < 0) fail = true;
    if (first_len > four_mb) fail = true;
    if (first_len == four_mb)
        if (theDispatcher->read(file, &test_over_flow, 1) > 0)
            fail = true;

    theDispatcher->close(file);

    if (fail) return NULL;
    return *(uint32_t**)(program_image_addr + 24);
}

} }
