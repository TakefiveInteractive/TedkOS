#include <inc/fs/filesystem.h>
#include <inc/fs/kiss.h>
#include <inc/fs/dev.h>

namespace filesystem {

const size_t MaxMountableFilesystems = 4;

/* Our singleton objects here */
Dispatcher dispatcher;
DevFS devFS;
KissFS kissFS;

Dispatcher::Dispatcher()
{
    this->_devFS = &devFS;
    this->_kissFS = &kissFS;
}

void Dispatcher::mountAll()
{
    _kissFS->init();
    _devFS->init();
}

}

