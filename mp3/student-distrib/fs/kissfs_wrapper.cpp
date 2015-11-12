#include <inc/fs/kiss_wrapper.h>

namespace filesystem {

void init_from_multiboot()
{
    static_cast<KissFS *>(theDispatcher->_kissFS)->init();
}

int32_t read_dentry(const uint8_t *fname, dentry_t *dentry)
{
    return static_cast<KissFS *>(theDispatcher->_kissFS)->readDentry(fname, dentry);
}

int32_t read_dentry(const uint32_t index, dentry_t* dentry)
{
    return static_cast<KissFS *>(theDispatcher->_kissFS)->readDentry(index, dentry);
}

int32_t read_data(const uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
    return static_cast<KissFS *>(theDispatcher->_kissFS)->readData(inode, offset, buf, length);
}

}

