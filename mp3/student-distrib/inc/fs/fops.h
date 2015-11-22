#ifndef _FOPS_H
#define _FOPS_H

#include <stddef.h>
#include <stdint.h>

// To avoid cyclic dependency, I have to do a dummy declaration
namespace filesystem {
    struct File {};
}

// By default every operation returns success and does nothing.
class IFOps
{
public:
    virtual int32_t open(filesystem::File& fdEntity);
    virtual int32_t close(filesystem::File& fdEntity);
    virtual int32_t read(filesystem::File& fdEntity, uint8_t *buf, int32_t bytes);
    virtual int32_t write(filesystem::File& fdEntity, const uint8_t *buf, int32_t bytes);
};

#endif /* _FOPS_H */
