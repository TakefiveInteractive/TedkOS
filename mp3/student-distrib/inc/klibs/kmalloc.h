#ifndef _KLIBS_KMALLOC_H_
#define _KLIBS_KMALLOC_H_

#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/bitset.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/maybe.h>

#ifdef __cplusplus
namespace memory {

template<size_t ElementSize, size_t PoolSize>
class ObjectPool {
    private:
        static constexpr size_t MaxNumElements = PoolSize / ElementSize;
        util::BitSet<MaxNumElements> freeMap;
        util::Stack<void *, MaxNumElements> freeStack;
        size_t toMapIndex(void *addr);
        size_t numValidElements;

    public:
        ObjectPool();
        Maybe<void*> get();
        bool empty() const;
        bool release(void* addr);

        void* operator new(size_t s, void* addr) { return addr; }
};

constexpr size_t operator "" _KB(unsigned long long len) { return len * 1024; }
constexpr size_t operator "" _MB(unsigned long long len) { return len * 1024 * 1024; }

template<size_t ElementSize>
constexpr size_t PageSizeOf =
    ([]() { static_assert(ElementSize == 16 || ElementSize == 256 || ElementSize == 8_KB || ElementSize == 256_KB, "Invalid page size"); })();
template<>
constexpr size_t PageSizeOf<16> = 4_MB;
template<>
constexpr size_t PageSizeOf<256> = 4_MB;
template<>
constexpr size_t PageSizeOf<8_KB> = 4_MB;
template<>
constexpr size_t PageSizeOf<256_KB> = 4_MB;

#endif

}

#endif
