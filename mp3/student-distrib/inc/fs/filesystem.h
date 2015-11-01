#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
namespace filesystem {

extern const size_t MaxMountableFilesystems;

class AbstractFS {
public:
    virtual void init() = 0;
};

class Dispatcher {
private:
    //PrefixTree lookup;
    AbstractFS *devFS;

public:
    AbstractFS *kissFS;  // this is public for cp2 cuz TAs have to test it

    Dispatcher();

    /*void mountAll();
    int32_t read(int32_t fd, void *buf, int32_t nbytes);
    int32_t write(int32_t fd, const void *buf, int32_t nbytes);
    int32_t open(const char *filename);
    int32_t close(int32_t fd);*/
};

extern Dispatcher dispatcher;

}
#endif

#endif

