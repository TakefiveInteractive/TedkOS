#include <inc/fs/dev.h>

namespace filesystem {

DevFS::DevFS()
{

}

void DevFS::init()
{

}

void DevFS::registerDevice(const char* path, FOpsGetter getterFn)
{
    Filename fn(path);
    deviceOfFilename.put(fn, getterFn);
}

bool DevFS::open(const char* filename, FsSpecificData *&fdData)
{
    bool found;
    auto fopsGetter = deviceOfFilename.get(Filename(filename), found);
    if (!found) return false;

    auto fopsInstance = fopsGetter();
    if (fopsInstance)
    {
        fdData = +fopsInstance;
        return true;
    }
    else
    {
        return false;
    }
}

int32_t DevFS::read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len)
{
    // Call FOpsReadImpl
    return reinterpret_cast<IFOps *>(data)->read(data, buf, len);
}

int32_t DevFS::write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len)
{
    // Call FOpsWriteImpl
    return reinterpret_cast<IFOps *>(data)->write(data, buf, len);
}

bool DevFS::close(FsSpecificData *fdData)
{
    IFOps *data = reinterpret_cast<IFOps *>(fdData);
    if (!data) return false;
    delete data;
    return true;
}

}

