#include <inc/fs/filesystem.h>
#include <inc/fs/dev_wrapper.h>

// C++ functions
namespace filesystem {
    void register_devfs(const char* path, const FOpsTable& jtable)
    {
        theDispatcher->register_devfs(path, jtable);
    }
}

// C functions
void register_devfs(const char* path, FOpsTable jtable)
{
    filesystem::theDispatcher->register_devfs(path, jtable);
}
