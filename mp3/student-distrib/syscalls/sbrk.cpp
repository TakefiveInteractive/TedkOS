#include "sbrk.h"
#include <inc/proc/tasks.h>
#include <inc/syscalls/syscalls.h>

namespace syscall { namespace sbrk {

int32_t syssbrk(int32_t incr)
{
    auto pd = getCurrentThreadInfo()->pcb.to_process;
    auto a = reinterpret_cast<int32_t>(pd->sbrk(incr));
    return a;
}

} }
