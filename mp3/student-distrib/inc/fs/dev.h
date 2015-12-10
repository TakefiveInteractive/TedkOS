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
    util::LinearProbingHashTable<37, Filename, FOpsGetter, HashFunc> deviceOfFilename;

public:
    DevFS();

    virtual void init();
    virtual bool open(const char* filename, FsSpecificData *&fdData);
    virtual bool close(FsSpecificData *fdData);
    virtual int32_t read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len);
    virtual int32_t write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len);
    virtual int32_t fstat(FsSpecificData *data, stat *st);
    virtual bool canSeek(FsSpecificData *fdData);
    virtual Maybe<uint32_t> getFileSize(FsSpecificData *fdData);
    void seekCallback(FsSpecificData *fdData, uint32_t newOffset) override;

    void registerDevice(const char* path, FOpsGetter getterFn);
};

}
#endif

#endif

