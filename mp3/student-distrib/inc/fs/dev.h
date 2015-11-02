#ifndef _DEV_H_
#define _DEV_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/fs/filesystem.h>
#include <inc/lphashtable.h>

/* Read a single character from the device */
char (*readFunc) ();

/* Write a single 32-bit integer to device
 * Return value: 0 = success, -1 = failure
 */
uint32_t (*writeFunc) (uint32_t data);

#ifdef __cplusplus
namespace filesystem {

struct JumpTable {
    readFunc read;
    writeFunc write;
}

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

