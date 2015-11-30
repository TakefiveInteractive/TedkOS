#include "sbrk.h"
#include <inc/proc/tasks.h>

namespace syscall { namespace sbrk {

int32_t syssbrk(int32_t incr)
{
    auto pd = getCurrentThreadInfo()->getProcessDesc();
    auto a = reinterpret_cast<int32_t>(pd->sbrk(incr));
    return a;
}

} }
