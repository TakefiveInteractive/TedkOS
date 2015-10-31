#include <inc/x86/filesystem_wrapper.h>

namespace filesystem {

void init_from_multiboot(multiboot_info *mbi)
{
    module_t* mod = (module_t*) mbi->mods_addr;
    kissFS.initFromMemoryAddress((uint8_t *)mod->mod_start, (uint8_t *)mod->mod_end);
}

int32_t read_dentry(const uint8_t *fname, dentry_t *dentry)
{
    return kissFS.readDentry(fname, dentry);
}

int32_t read_dentry(const uint32_t index, dentry_t* dentry)
{
    return kissFS.readDentry(index, dentry);
}

int32_t read_data(const uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length)
{
    return kissFS.readData(inode, offset, buf, length);
}

}

/* C interface */

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
    return filesystem::read_dentry(fname, dentry);
}

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
{
    return filesystem::read_dentry(index, dentry);
}

int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    return filesystem::read_data(inode, offset, buf, length);
}

