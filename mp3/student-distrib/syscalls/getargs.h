#ifndef _GETARGS_H_
#define _GETARGS_H_

#include <stddef.h>
#include <stdint.h>

namespace syscall { namespace exec {

int32_t getargs (uint8_t* buf, int32_t nbytes);

} }

#endif
