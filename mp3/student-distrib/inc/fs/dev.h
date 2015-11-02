#ifndef _FS_DEV_H_
#define _FS_DEV_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/fs/filesystem.h>
#include <inc/lphashtable.h>

/* Read a single character from the device */
typedef int32_t (*readFunc) (uint8_t *buf, int32_t bytes);

/* Write a single character to device */
typedef int32_t (*writeFunc) (const uint8_t *buf, int32_t bytes);

#ifdef __cplusplus
namespace filesystem {

struct JumpTable {
    readFunc read;
    writeFunc write;
};

class DevFS : public AbstractFS {
private:
    util::LinearProbingHashTable<37, Filename, JumpTable, HashFunc> deviceOfFilename;

public:
    DevFS();

    virtual void init();
    virtual bool open(const char* filename, FsSpecificData *fdData);
    virtual int32_t read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len);
    virtual int32_t write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len);

    void registerDevice(const char* path, readFunc reader, writeFunc writer);
};

}
#endif

#endif

