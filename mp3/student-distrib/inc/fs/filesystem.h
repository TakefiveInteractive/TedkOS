#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <stddef.h>
#include <stdint.h>

#include <inc/fs/fops.h>
#include <inc/klibs/prefix_tree.h>
#include <inc/klibs/fixedmemorypool.h>
#include <inc/fs/stat.h>

#ifdef __cplusplus
namespace filesystem {

extern const size_t MaxMountableFilesystems;

static const uint32_t MaxFilenameLength = 32;
/* Class holding a filename that implements a proper equal operator */
class Filename {
private:
    const char *str;
public:
    Filename(const char *s) { this->str = s; }

    Filename() { this->str = nullptr; }

    bool operator == (const Filename& rhs) const {
        return strncmp(str, rhs.str, MaxFilenameLength) == 0;
    }

    bool operator != (const Filename& rhs) const {
        return !(*this == rhs);
    }

    const char operator [] (size_t idx) const { return str[idx]; };
};

struct HashFunc {
    static uint32_t hash(const Filename& s)
    {
        uint32_t hash = 0;
        uint32_t used = 0;
        while (s[used] && used < MaxFilenameLength)
        {
            hash = hash * 101 + s[used];
            used++;
        }
        return hash;
    }
};

class AbstractFS;

struct File {
    size_t offset;
    AbstractFS *fs;
    FsSpecificData *fsSpecificData;
};

class AbstractFS {
public:
    virtual void init() = 0;
    virtual bool open(const char* filename, FsSpecificData *&fdData) = 0;
    virtual bool close(FsSpecificData *fdData) = 0;
    virtual int32_t read(FsSpecificData *data, uint32_t offset, uint8_t *buf, uint32_t len) = 0;
    virtual int32_t write(FsSpecificData *data, uint32_t offset, const uint8_t *buf, uint32_t len) = 0;
    virtual int32_t fstat(FsSpecificData *data, stat *st) = 0;
};

class Dispatcher {
private:
    util::PrefixTree<AbstractFS *> lookup;
    AbstractFS *_kissFS;
    AbstractFS *_devFS;

    void mount(AbstractFS *fs, const char *path);

public:
    Dispatcher();

    void mountAll();

    int32_t read(File &fd, void *buf, int32_t nbytes);
    int32_t write(File &fd, const void *buf, int32_t nbytes);
    bool open(File &fd, const char *filename);
    bool close(File &fd);
    int32_t fstat(File &fd, stat *st);

    void register_devfs(const char* path, FOpsGetter getterFn);

    static void init();
};

extern Dispatcher *theDispatcher;

}
#endif

#endif
