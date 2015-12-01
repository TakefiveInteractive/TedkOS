#include "getargs.h"
#include <inc/proc/tasks.h>
#include <inc/syscalls/syscalls.h>

namespace syscall { namespace exec {

using namespace filesystem;
using syscall::validUserPointer;

int32_t getargs (uint8_t* buf, int32_t nbytes)
{
    if(!validUserPointer(buf))
        return -1;

    auto pd = getCurrentThreadInfo()->getProcessDesc();
    if (!pd->arg) { buf[0] = '\0'; return 0; }
    int32_t arg_len = strlen(pd->arg);
    if (nbytes < arg_len + 1) return -1;
    memcpy(buf, pd->arg, arg_len);
    buf[arg_len] = '\0';
    return 0;
}

} }
