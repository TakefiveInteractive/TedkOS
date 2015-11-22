#include <inc/fs/filesystem.h>
#include <inc/fs/dev_wrapper.h>

// C++ functions
namespace filesystem {
    void register_devfs(const char* path, IFOps* jtable)
    {
        theDispatcher->register_devfs(path, jtable);
    }
}
