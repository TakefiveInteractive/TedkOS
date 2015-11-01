#include <inc/fs/filesystem.h>
#include <inc/fs/kiss.h>

namespace filesystem {

const size_t MaxMountableFilesystems = 4;

// DevFS _devFS;
KissFS _kissFS;
Dispatcher::Dispatcher()
{
    this->kissFS = &_kissFS;
}

/* Our singleton object here */
Dispatcher dispatcher;

}

