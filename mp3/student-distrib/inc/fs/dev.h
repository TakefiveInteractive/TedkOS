#ifndef _FS_DEV_H_
#define _FS_DEV_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/fs/filesystem.h>
#include <inc/klibs/lphashtable.h>
#include <inc/fs/fops.h>

#ifdef __cplusplus
namespace filesystem {

class DevFS : public AbstractFS {
private:
    util::LinearProbingHashTable<37, Filename, IFOps*, HashFunc> deviceOfFilename;

public:
    DevFS();

    virtual void init();
    virtual bool open(File& fd, FsSpecificData *fdData, const char* filename);
    virtual bool close(File& fd, FsSpecificData *fdData);
    virtual int32_t read(File& fd, FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len);
    virtual int32_t write(File& fd, FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len);

    void registerDevice(const char* path, IFOps* jtable);
};

}
#endif

#endif

