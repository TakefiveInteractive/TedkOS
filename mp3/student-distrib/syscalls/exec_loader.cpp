#include "exec.h"
#include <inc/error.h>

using namespace syscall_exec;

//   Load "file", an executable, to "program_image_addr"
//   You can assume that CR3 already points the page table of this process.
// Return 0 if success. Return -ENOMEM if file is too large (not possible, though)
int32_t kiss_loader(const uint8_t* file)
{
    return 0;
}
