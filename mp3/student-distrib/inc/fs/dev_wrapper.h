#ifndef _DEVSIDE_WRAPPER_H_
#define _DEVSIDE_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/fs/fops.h>

namespace filesystem {
    void register_devfs(const char* path, IFOps* jtable);
}

#endif
