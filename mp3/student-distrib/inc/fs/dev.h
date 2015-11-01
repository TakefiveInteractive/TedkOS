#ifndef _DEV_H_
#define _DEV_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/fs/filesystem.h>

#ifdef __cplusplus
namespace filesystem {

class DevFS : public AbstractFS {
public:
    DevFS();
    void init();
};

}
#endif

#endif

