#include <inc/fs/dev.h>

namespace filesystem {

DevFS::DevFS()
{

}

void DevFS::init()
{

}

bool DevFS::open(const char* filename, FsSpecificData *data)
{
    return true;
}

int32_t DevFS::read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len)
{
    return -1;
}

int32_t DevFS::write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len)
{
    return -1;
}

}

