#ifndef _SYSCALL_SBRK_H_
#define _SYSCALL_SBRK_H_

#include <stdint.h>
#include <stddef.h>

namespace syscall { namespace sbrk {

int32_t syssbrk(int32_t incr);

} }

#endif /* _SYSCALL_HALT */

