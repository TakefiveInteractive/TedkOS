#include "getargs.h"
#include <inc/proc/tasks.h>

namespace syscall { namespace exec {

int32_t getargs (uint8_t* buf, int32_t nbytes)
{
    auto pd = getCurrentThreadInfo()->pcb.to_process;
    if (!pd->arg) return -1;
    int32_t arg_len = strlen(pd->arg);
    if (nbytes < arg_len + 1) return -1;
    memcpy(buf, pd->arg, arg_len);
    buf[arg_len] = '\0';
    return 0;
}

} }
