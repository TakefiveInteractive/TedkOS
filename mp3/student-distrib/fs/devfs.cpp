#include <inc/fs/dev.h>

namespace filesystem {

DevFS::DevFS()
{

}

void DevFS::init()
{

}

void DevFS::registerDevice(const char* path, readFunc reader, writeFunc writer)
{
    Filename fn(path);
    JumpTable jtable = { .read = reader, .write = writer };
    deviceOfFilename.put(fn, jtable);
}

bool DevFS::open(const char* filename, FsSpecificData *data)
{
    bool found;
    JumpTable jtable = deviceOfFilename.get(Filename(filename), found);
    if (!found) return false;

    data->jtable = jtable;
    return true;
}

int32_t DevFS::read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len)
{
    return data->jtable.read(buf, len);
}

int32_t DevFS::write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len)
{
    return data->jtable.write(buf, len);
}

}

