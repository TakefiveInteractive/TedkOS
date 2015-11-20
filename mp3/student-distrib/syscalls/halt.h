#ifndef _SYSCALL_HALT
#define _SYSCALL_HALT

#include <stdint.h>
#include <stddef.h>
namespace syscall { namespace halt {

int32_t syshalt(uint32_t retval);

} }

#endif /* _SYSCALL_HALT */
