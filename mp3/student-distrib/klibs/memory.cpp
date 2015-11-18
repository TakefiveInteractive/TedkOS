#include <inc/klibs/memory.h>

namespace memory {

uint32_t ceil(uint32_t len, uint32_t blkSize)
{
    uint32_t remainder = len % blkSize;
    if (remainder > 0) return len / blkSize + 1;
    return len / blkSize;
}

}

