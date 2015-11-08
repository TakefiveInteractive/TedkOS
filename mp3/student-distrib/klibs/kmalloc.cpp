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
    auto addrval = reinterpret_cast<uint32_t>(addr);
    auto thisval = reinterpret_cast<uint32_t>(this);
    // Not in our pool
    if (addrval < thisval) return MaxNumElements + 1;
    const size_t offset = addrval - thisval;
    return offset / ElementSize;
}

template<size_t ElementSize, size_t PoolSize>
bool ObjectPool<ElementSize, PoolSize>::release(void *addr)
{
    auto idx = toMapIndex(addr);
    if (freeMap.test(idx))
    {
        freeMap.clear(idx);
        freeStack.push(addr);
        return true;
    }
    return false;
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

namespace KMemory {

static constexpr size_t MaxPoolNum = 8;
util::Stack<ObjectPool<16, PageSizeOf<16>> *, MaxPoolNum> pools16;
util::Stack<ObjectPool<256, PageSizeOf<256>> *, MaxPoolNum> pools256;
util::Stack<ObjectPool<8_KB, PageSizeOf<8_KB>> *, MaxPoolNum> pools8K;

template<size_t ElementSize>
class PoolGetter {
public:
    static const auto val()
    {
        static_assert(ElementSize == 16 || ElementSize == 256 || ElementSize == 8_KB, "Invalid page size");
    }
};

template<> class PoolGetter<16> { public: static const auto val() { return pools16; } };
template<> class PoolGetter<256> { public: static const auto val() { return pools256; } };
template<> class PoolGetter<8_KB> { public: static const auto val() { return pools8K; } };

template<size_t ElementSize> Maybe<void *> paraAllocate();
template<size_t ElementSize> void paraFree(void *addr);

Maybe<void *> allocImpl(size_t size);
void freeImpl(void *addr, size_t size);

template<size_t ElementSize>
Maybe<void *> paraAllocate()
{
    auto pools = PoolGetter<ElementSize>::val();
    auto x = pools.first([](auto pool) { return pool->get(); });
    if (x)
    {
        return x;
    }
    else
    {
        // Allocate another pool?
        if (pools.full())
        {
            return Maybe<void *>();
        }
        else
        {
            // TODO: move this to a new page!
            ObjectPool<ElementSize, PageSizeOf<ElementSize>> newPool;
            return newPool.get();
        }
    }
}

template<size_t ElementSize>
void paraFree(void *addr)
{
    auto pools = PoolGetter<ElementSize>::val();
    pools.firstTrue(addr, [](auto pool, void* addr) { return pool->release(addr); });
}

Maybe<void *> allocImpl(size_t size)
{
    if (size <= 16)
    {
        return paraAllocate<16>();
    }
    else if (size <= 256)
    {
        return paraAllocate<256>();
    }
    else if (size <= 8_KB)
    {
        return paraAllocate<8_KB>();
    }
    else
    {
        return Maybe<void *>();
    }
}

void freeImpl(void *addr, size_t size)
{
    if (size <= 16)
    {
        paraFree<16>(addr);
    }
    else if (size <= 256)
    {
        paraFree<256>(addr);
    }
    else if (size <= 8_KB)
    {
        paraFree<8_KB>(addr);
    }
    else
    {
        // Trigger an exception
        __asm__ __volatile__("int $25;" : : );
    }

}

template<typename T>
T* alloc()
{
    return reinterpret_cast<T*>(KMemory::allocImpl(sizeof(T)));
}

template<typename T>
void free(T* addr)
{
    KMemory::freeImpl(reinterpret_cast<void *>(addr), sizeof(T));
}

}

};

