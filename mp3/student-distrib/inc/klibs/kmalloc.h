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
        void release(void* addr);
};

constexpr size_t operator "" _KB(unsigned long long len) { return len * 1024; }
constexpr size_t operator "" _MB(unsigned long long len) { return len * 1024 * 1024; }

class KMemory {
    private:
        static constexpr size_t MaxPoolNum = 8;
        util::Stack<ObjectPool<16, 4_KB> *, MaxPoolNum> pool16;
        util::Stack<ObjectPool<256, 4_KB> *, MaxPoolNum> pool256;
        util::Stack<ObjectPool<8_KB, 4_MB> *, MaxPoolNum> pool8K;
    public:
        template<typename T> T* alloc();
        template<typename T> void free(T* addr);
        void* allocImpl(size_t size);
        void freeImpl(void *addr);
};
#endif

}

#endif
