#ifndef _INC_KLIBS_MEMORY_H_
#define _INC_KLIBS_MEMORY_H_

#include <stdint.h>
#include <stddef.h>

namespace memory {

constexpr size_t operator "" _KB(unsigned long long len) { return len * 1024; }
constexpr size_t operator "" _MB(unsigned long long len) { return len * 1024 * 1024; }

template<size_t N>
static inline size_t alignAt(size_t x)
{
    return ~(N - 1) & (x - 1 + N);
}

template<size_t N, size_t x>
constexpr size_t align = ~(N - 1) & (x - 1 + N);

uint32_t ceil(uint32_t len, uint32_t blkSize);

/**
 * Converts a virtual address to physical one.
 */
uint32_t virtualAddressToPhysicalAddress(void* ptr);

}

#endif
