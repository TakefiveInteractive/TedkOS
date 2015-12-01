#ifndef _INC_SYSCALLS_H_
#define _INC_SYSCALLS_H_

#include <../shared/ece391sysnum.h>

void systemCallHandler(void);

namespace syscall
{
bool validUserPointer(void* ptr);
}

#endif
