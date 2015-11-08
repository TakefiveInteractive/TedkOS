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
#endif

}

#endif
