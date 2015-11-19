#include "sbrk.h"
#include <inc/proc/tasks.h>

namespace syscall { namespace sbrk {

int32_t syssbrk(int32_t incr)
{
    printf("Sbrk! incr = %d\n", incr);
    auto pd = getCurrentThreadInfo()->pcb.to_process;
    auto a = reinterpret_cast<int32_t>(pd->sbrk(incr));
    printf("Addr: %#x\n", a);
    return a;
}

} }
