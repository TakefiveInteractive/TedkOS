#include <inc/syscalls/exec.h>
#include <inc/fs/kiss.h>
#include <inc/fs/filesystem.h>

using namespace filesystem;

namespace syscall { namespace exec {

static const int NORMAL_FILE = 2;
/*
 *  MagicNum: 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46.
 */

// Check whether exists, and whether executable.
// Return 0 if not exist or not an executable.
int8_t is_kiss_executable(const boost::unique_ptr<char[]> &filename)
{
    File file;
    if (!theDispatcher->open(file, filename.get())) return 0;

    stat stats = {};
    theDispatcher->fstat(file, &stats);
    if (!S_ISREG(stats.st_mode)) // not regular file
    {
        theDispatcher->close(file);
        return 0;
    }

    uint8_t buf[4] = {};
    uint32_t len = theDispatcher->read(file, buf, sizeof(buf));
    theDispatcher->close(file);

    if (len < 4) // read error, or input file length error
        return 0;
    if (buf[0] == 0x7f &&
        buf[1] == 0x45 &&
        buf[2] == 0x4c &&
        buf[3] == 0x46
    )
        return 1;
    else
        return 0;
}

} }
