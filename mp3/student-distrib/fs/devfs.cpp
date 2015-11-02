#include <inc/fs/dev.h>

namespace filesystem {

DevFS::DevFS()
{

}

void DevFS::init()
{

}

void DevFS::registerDevice(const char* path, const FOpsTable& jtable)
{
    Filename fn(path);
    deviceOfFilename.put(fn, jtable);
}

bool DevFS::open(const char* filename, FsSpecificData *data)
{
    bool found;
    FOpsTable jtable = deviceOfFilename.get(Filename(filename), found);
    if (!found) return false;

    // Call FOpsOpenImpl, currently we do not have fdEntry structure (=NULL)
    if (jtable.open(NULL) != 0)
        return false;

    data->jtable = jtable;
    return true;
}

int32_t DevFS::read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len)
{
    // Call FOpsReadImpl, currently we do not have fdEntry structure (=NULL)
    return data->jtable.read(NULL, buf, len);
}

int32_t DevFS::write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len)
{
    // Call FOpsWriteImpl currently we do not have fdEntry structure (=NULL)
    return data->jtable.write(NULL, buf, len);
}

bool DevFS::close(const char* filename, FsSpecificData *data)
{
    // TODO: deallocate FsSpecificData, if possible.
    return data->jtable.close(NULL) == 0;
}

}

