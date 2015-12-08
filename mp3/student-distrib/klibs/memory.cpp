#include <inc/klibs/memory.h>
#include <inc/klibs/palloc.h>

namespace memory {

uint32_t ceil(uint32_t len, uint32_t blkSize)
{
    uint32_t remainder = len % blkSize;
    if (remainder > 0) return len / blkSize + 1;
    return len / blkSize;
}

uint32_t virtualAddressToPhysicalAddress(void* ptr)
{
    return (palloc::cpu0_memmap.translate(ptr).pde & ALIGN_4MB_ADDR) | (((uint32_t)ptr) & (~ALIGN_4MB_ADDR));
}

}

