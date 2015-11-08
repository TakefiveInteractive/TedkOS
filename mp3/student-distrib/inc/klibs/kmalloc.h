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
        static constexpr MaxNumElements = PoolSize / ElementSize;
        util::BitSet<MaxNumElements> freeMap;
        util::Stack<void *, MaxNumElements> freeStack;

    public:
        ObjectPool();
        Maybe<void*> get();
        void release(void* addr);
};

constexpr size_t operator "" KB(size_t len) { return len * 1024; }
constexpr size_t operator "" MB(size_t len) { return len * 1024 * 1024; }

class KMemory {
    private:
        static constexpr MaxPoolNum = 8;
        util::Stack<ObjectPool<16, 4KB> *, MaxPoolNum> pool16;
        util::Stack<ObjectPool<256, 4KB> *, MaxPoolNum> pool256;
        util::Stack<ObjectPool<8KB, 4MB> *, MaxPoolNum> pool8K;
    public:
        template<typename T> T* alloc();
        template<typename T> void free(T* addr);
        void* allocImpl();
        void freeImpl(void *addr);
};
#endif

}

#endif
