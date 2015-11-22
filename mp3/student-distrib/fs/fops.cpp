#include <inc/fs/fops.h>

using namespace filesystem;

virtual int32_t IFOps::open(File& fdEntity)
{
    return 0;
}

virtual int32_t IFOps::close(File& fdEntity)
{
    return 0;
}

virtual int32_t IFOps::read(File& fdEntity, uint8_t *buf, int32_t bytes)
{
    return 0;
}

virtual int32_t IFOps::write(File& fdEntity, const uint8_t *buf, int32_t bytes)
{
    return 0;
}

