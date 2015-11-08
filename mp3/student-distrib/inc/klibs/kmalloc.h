#ifndef _KLIBS_KMALLOC_H_
#define _KLIBS_KMALLOC_H_

#include <stdint.h>
#include <stddef.h>
#include <inc/bitset.h>
#include <inc/stack.h>
#include <inc/maybe.h>

#ifdef __cplusplus
namespace memory {

template<size_t ElementSize, size_t PoolSize>
class ObjectPool {
    private:
        static constexpr size_t MaxNumElements = PoolSize / ElementSize;
        util::BitSet<MaxNumElements> freeMap;
        util::Stack<void *, MaxNumElements> freeStack;
        size_t toMapIndex(void *addr);

    public:
        ObjectPool();
        Maybe<void*> get();
        bool release(void* addr);
};

constexpr size_t operator "" _KB(unsigned long long len) { return len * 1024; }
constexpr size_t operator "" _MB(unsigned long long len) { return len * 1024 * 1024; }

template<size_t ElementSize>
constexpr size_t PageSizeOf =
    ([]() { static_assert(ElementSize == 16 || ElementSize == 256 || ElementSize == 8_KB, "Invalid page size"); })();
template<>
constexpr size_t PageSizeOf<16> = 4_KB;
template<>
constexpr size_t PageSizeOf<256> = 4_KB;
template<>
constexpr size_t PageSizeOf<8_KB> = 4_MB;

namespace KMemory {
    template<typename T> T* alloc();
    template<typename T> void free(T* addr);
};
#endif

}

#endif
