#ifndef _SYS_VIDMAP_H_
#define _SYS_VIDMAP_H_

#include <stddef.h>
#include <stdint.h>

namespace syscall
{
int32_t vidmap (uint8_t** screen_start);
}

#endif
