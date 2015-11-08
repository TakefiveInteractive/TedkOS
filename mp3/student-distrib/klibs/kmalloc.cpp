#include <inc/klibs/kmalloc.h>

namespace memory {

template<size_t N>
static inline size_t alignAt(size_t x)
{
    return ~(N - 1) & (x - 1 + N);
}

template<size_t N, size_t x>
constexpr size_t align = ~(N - 1) & (x - 1 + N);

template<size_t ElementSize, size_t PoolSize>
Maybe<void *> ObjectPool<ElementSize, PoolSize>::get()
{
    if (freeStack.empty())
    {
        return Maybe<void *>();
    }
    else
    {
        void *addr = freeStack.pop();
        freeMap.clear(toMapIndex(addr));
        return Maybe<void *>(addr);
    }
}

template<size_t ElementSize, size_t PoolSize>
size_t ObjectPool<ElementSize, PoolSize>::toMapIndex(void *addr)
{
    const size_t offset = reinterpret_cast<uint8_t *>(addr) - reinterpret_cast<uint8_t *>(this);
    return offset / ElementSize;
}

template<size_t ElementSize, size_t PoolSize>
void ObjectPool<ElementSize, PoolSize>::release(void *addr)
{
    auto idx = toMapIndex(addr);
    if (freeMap.test(idx))
    {
        freeMap.clear(idx);
        freeStack.push(addr);
    }
}

template<size_t ElementSize, size_t PoolSize>
ObjectPool<ElementSize, PoolSize>::ObjectPool()
{
    constexpr size_t ourSize = align<ElementSize, sizeof(ObjectPool)>;
    constexpr size_t ourNumElements = ourSize / ElementSize;
    // Mark ourself as occupied, and rest are free
    for (size_t i = 0; i < ourNumElements; i++)
    {
        freeMap.clear(i);
    }
    for (size_t i = ourNumElements; i < MaxNumElements; i++)
    {
        freeMap.set(i);
        freeStack.push(reinterpret_cast<void *>(reinterpret_cast<uint8_t *>(this) + ElementSize * i));
    }
}

template<typename T>
T* KMemory::alloc()
{
    return reinterpret_cast<T*>(allocImpl(sizeof(T)));
}

template<typename T>
void KMemory::free(T* addr)
{
    freeImpl(reinterpret_cast<void *>(addr));
}


void* KMemory::allocImpl(size_t size)
{
}

void KMemory::freeImpl(void *addr)
{
}

};

