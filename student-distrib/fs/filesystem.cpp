#include <inc/fs/filesystem.h>
#include <inc/fs/kiss.h>
#include <inc/fs/dev.h>

namespace filesystem {

const size_t MaxMountableFilesystems = 4;

/* Our singleton object here */
Dispatcher *theDispatcher = nullptr;

Dispatcher::Dispatcher()
{
    this->_devFS = new DevFS();
    this->_kissFS = new KissFS();
}

void Dispatcher::init()
{
    if (theDispatcher != nullptr) return;
    theDispatcher = new Dispatcher();
    theDispatcher->mountAll();
}

void Dispatcher::mountAll()
{
    _kissFS->init();
    _devFS->init();

    mount(_kissFS, "/");
    mount(_devFS, "/dev/");
}

void Dispatcher::mount(AbstractFS *fs, const char *path)
{
    lookup.insert(path, fs);
}

int32_t Dispatcher::read(File &fd, void *buf, int32_t nbytes)
{
    int32_t result = fd.fs->read(fd.fsSpecificData, fd.offset, static_cast<uint8_t*>(buf), nbytes);
    if (result == -1) return -1;
    fd.offset += result;
    return result;
}

int32_t Dispatcher::write(File &fd, const void *buf, int32_t nbytes)
{
    int32_t result = fd.fs->write(fd.fsSpecificData, fd.offset, static_cast<const uint8_t*>(buf), nbytes);
    if (result == -1) return -1;
    fd.offset += result;
    return result;
}

int32_t Dispatcher::fstat(File &fd, stat *st)
{
    return fd.fs->fstat(fd.fsSpecificData, st);
}

int32_t Dispatcher::lseek(File &fd, int32_t offset, int32_t whence)
{
    if (!fd.fs->canSeek(fd.fsSpecificData)) return -1;
    auto fileSize = fd.fs->getFileSize(fd.fsSpecificData);
    switch (whence)
    {
    case SEEK_SET:
        fd.offset = offset;
        break;
    case SEEK_CUR:
        fd.offset += offset;
        break;
    case SEEK_END:
        if (!fileSize) return -1;
        fd.offset = +fileSize + offset;
        break;
    default:
        return -1;
    }
    fd.fs->seekCallback(fd.fsSpecificData, fd.offset);
    return fd.offset;
}

bool Dispatcher::open(File &fd, const char *filename)
{
    const char *fn = filename;
    // Patch: support accessing rtc without fs root
    if (strncmp(fn, "rtc", 4) == 0) fn = "/dev/rtc";
    // Patch #2: append slash in front of raw path
    char goodName[70] = { '/', '\0' };
    if (fn[0] != '/')
    {
        strncpy(&goodName[1], fn, 69);
        fn = goodName;
    }

    auto x = lookup.search(fn);
    if (x.val == nullptr)
    {
        return false;
    }
    else
    {
        AbstractFS *_fs = x.val;
        fd = { .offset = 0, .fs = _fs };
        // Skip over x.len characters, trimming the mount point from fs
        bool result = _fs->open(fn + x.len, fd.fsSpecificData);
        if (!result)
            return false;
        return true;
    }
}

bool Dispatcher::close(File &fd)
{
    bool result = fd.fs->close(fd.fsSpecificData);
    if (!result)
        return false;
    return true;
}

void Dispatcher::register_devfs(const char* path, FOpsGetter getterFn)
{
   static_cast<DevFS *>(theDispatcher->_devFS)->registerDevice(path, getterFn);
}

}
