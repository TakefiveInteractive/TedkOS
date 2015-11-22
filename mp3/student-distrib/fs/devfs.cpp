#include <inc/fs/dev.h>

namespace filesystem {

DevFS::DevFS()
{

}

void DevFS::init()
{

}

void DevFS::registerDevice(const char* path, IFOps* jtable)
{
    Filename fn(path);
    deviceOfFilename.put(fn, jtable);
}

bool DevFS::open(File& fd, FsSpecificData *fdData, const char* filename)
{
    bool found;
    auto jtable = deviceOfFilename.get(Filename(filename), found);
    if (!found) return false;

    // Call FOpsOpenImpl, currently we do not have fdEntry structure (=NULL)
    if (jtable->open(fd) != 0)
        return false;

    data->jtable = jtable;
    return true;
}

int32_t DevFS::read(File& fd, FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len)
{
    // Call FOpsReadImpl, currently we do not have fdEntry structure (=NULL)
    return data->jtable->read(fd, buf, len);
}

int32_t DevFS::write(File& fd, FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len)
{
    // Call FOpsWriteImpl currently we do not have fdEntry structure (=NULL)
    return data->jtable->write(fd, buf, len);
}

bool DevFS::close(File& fd, FsSpecificData *data)
{
    // TODO: deallocate FsSpecificData, if possible.
    return data->jtable->close(fd) == 0;
}

}

