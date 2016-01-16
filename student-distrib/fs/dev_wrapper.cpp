#include <inc/fs/filesystem.h>
#include <inc/fs/dev_wrapper.h>

// C++ functions
namespace filesystem {
    void register_devfs(const char* path, FOpsGetter fn)
    {
        theDispatcher->register_devfs(path, fn);
    }
}
