#include "sbrk.h"
#include <inc/proc/tasks.h>

namespace syscall { namespace sbrk {

int32_t syssbrk(int32_t incr)
{
    printf("Sbrk!\n");
    auto pd = getCurrentThreadInfo()->pcb.to_process;
    return reinterpret_cast<int32_t>(pd->sbrk(incr));
}

} }
