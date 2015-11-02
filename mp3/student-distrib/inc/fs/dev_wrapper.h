#ifndef _DEVSIDE_WRAPPER_H_
#define _DEVSIDE_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/fs/fops.h>


#ifdef __cplusplus
namespace filesystem {
    void register_devfs(const char* path, const FOpsTable& jtable);
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
// C-side functions
void register_devfs(const char* path, FOpsTable jtable);

#ifdef __cplusplus
}
#endif

#endif
