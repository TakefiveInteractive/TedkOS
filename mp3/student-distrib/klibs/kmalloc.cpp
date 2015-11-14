#include <inc/klibs/kmalloc.h>
#include <inc/klibs/palloc.h>
#include <inc/x86/paging.h>
#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/AutoSpinLock.h>

using namespace palloc;

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
bool ObjectPool<ElementSize, PoolSize>::empty() const
{
    return freeStack.size() == numValidElements;
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
    if (idx >= MaxNumElements) return false;
    if (!freeMap.test(idx))
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
    numValidElements = MaxNumElements - ourNumElements;
}

namespace KMemory {

static constexpr size_t MaxPoolNumInSingleSlot = 20;
static constexpr size_t MaxPoolNumInAllSlots = 20;
size_t totalNumPools = 0;

template<size_t Size> using PoolType = ObjectPool<Size, PageSizeOf<Size>>;

util::Stack<ObjectPool<16, PageSizeOf<16>> *, MaxPoolNumInSingleSlot> pools16;
util::Stack<ObjectPool<256, PageSizeOf<256>> *, MaxPoolNumInSingleSlot> pools256;
util::Stack<ObjectPool<8_KB, PageSizeOf<8_KB>> *, MaxPoolNumInSingleSlot> pools8K;
util::Stack<ObjectPool<256_KB, PageSizeOf<256_KB>> *, MaxPoolNumInSingleSlot> pools256K;

template<size_t ElementSize>
class PoolGetter { };

template<> class PoolGetter<16> { public: static const auto val() { return &pools16; } };
template<> class PoolGetter<256> { public: static const auto val() { return &pools256; } };
template<> class PoolGetter<8_KB> { public: static const auto val() { return &pools8K; } };
template<> class PoolGetter<256_KB> { public: static const auto val() { return &pools256K; } };

template<size_t ElementSize>
Maybe<void *> paraFindAndReleaseFreePool()
{
    auto slot = PoolGetter<ElementSize>::val();
    size_t idx = 0;
    auto x = slot->template first<PoolType<ElementSize> *>(idx, [](auto pool) {
        if (pool->empty())
        {
            return Maybe<PoolType<ElementSize> *>(pool);
        }
        else
        {
            return Maybe<PoolType<ElementSize> *>();
        }
    });
    if (x)
    {
        // Free it
        slot->drop(idx);
        totalNumPools--;
        (!x)->~PoolType<ElementSize>();
        return Maybe<void *>(!x);
    }
    return Maybe<void *>();
}

Maybe<void *> findAndReleaseFreePool()
{
    return paraFindAndReleaseFreePool<16>()
        >> [](){ return paraFindAndReleaseFreePool<256>(); }
        >> [](){ return paraFindAndReleaseFreePool<8_KB>(); }
        >> [](){ return paraFindAndReleaseFreePool<256_KB>(); };
}

template<size_t ElementSize>
Maybe<void *> paraAllocate()
{
    auto slot = PoolGetter<ElementSize>::val();
    auto x = slot->template first<void *>([](auto pool) { return pool->get(); });
    if (x)
    {
        return x;
    }
    else    // Our slot have no empty pools
    {
        // Allocate another pool?
        if (slot->full())
        {
            return Maybe<void *>();
        }
        else
        {
            // Identify some single empty pool in other slots, and reuse if possible
            auto freePool = findAndReleaseFreePool();
            if (freePool)
            {
                auto newPool = new (!freePool) PoolType<ElementSize>();
                slot->push(newPool);
                return newPool->get();
            }
            else
            {
                if (totalNumPools == MaxPoolNumInAllSlots) return Maybe<void *>();

                auto physAddr = physPages.allocPage(1);
                void* addr = virtLast1G.allocPage(1);

                if(!commonMemMap.add(VirtAddr(addr), PhysAddr(physAddr, PG_WRITABLE)))
                    return Maybe<void *>();
                if(!spareMemMaps[currProcMemMap].add(VirtAddr(addr), PhysAddr(physAddr, PG_WRITABLE)))
                    ; // kill the process.
                if(!spareMemMaps[currProcMemMap].isLoadedToCR3(&cpu0_paging_lock))
                {
                    AutoSpinLock lock(&cpu0_paging_lock);
                    LOAD_4MB_PAGE((uint32_t)addr >> 22, (uint32_t)physAddr << 22, PG_WRITABLE);
                }
                {
                    AutoSpinLock lock(&cpu0_paging_lock);
                    RELOAD_CR3();
                }

                auto newPool = new (addr) PoolType<ElementSize>();
                slot->push(newPool);
                totalNumPools++;
                return newPool->get();
            }
        }
    }
}

template<size_t ElementSize>
bool paraFree(void *addr)
{
    auto pools = PoolGetter<ElementSize>::val();
    size_t idx;
    bool success = pools->firstTrue(addr, idx, [](auto pool, void* addr) { return pool->release(addr); });
    if (success)
    {
        // See if we can drop this block
        if (pools->get(idx)->empty())
        {
            // Do nothing here
            /*auto poolAddr = pools->drop(idx);
            uint32_t pdIndex = ((uint32_t)poolAddr >> 22);
            uint32_t physAddr = global_cr3val[pdIndex] & 0xffc00000;
            poolAddr->~ObjectPool<ElementSize, PageSizeOf<ElementSize>>();
            physPages.freePage(physAddr >> 22);
            virtLast1G.freePage(poolAddr);
            global_cr3val[pdIndex] = 0;
            RELOAD_CR3();*/
        }
    }
    return success;
}

Maybe<void *> allocImpl(size_t size)
{
    //printf("size: %d\n", size);
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
    else if (size <= 256_KB)
    {
        return paraAllocate<256_KB>();
    }
    else
    {
        return Maybe<void *>();
    }
}

void freeImpl(void *addr)
{
    if (!paraFree<16>(addr) && !paraFree<256>(addr) && !paraFree<8_KB>(addr) && !paraFree<256_KB>(addr))
    {
        // Trigger an exception
        __asm__ __volatile__("int $25;" : : );
    }
}

bool freeImpl(void *addr, size_t size)
{
    if (size <= 16)
    {
        return paraFree<16>(addr);
    }
    else if (size <= 256)
    {
        return paraFree<256>(addr);
    }
    else if (size <= 8_KB)
    {
        return paraFree<8_KB>(addr);
    }
    else if (size <= 256_KB)
    {
        return paraFree<256_KB>(addr);
    }
    else
    {
        return false;
    }

}

}

};

void* operator new(size_t s) {
    auto x = memory::KMemory::allocImpl(s);
    if (x)
    {
        return !x;
    }
    else
    {
        // Trigger an exception
        __asm__ __volatile__("int $24;" : : );
        return NULL;
    }
}

void* operator new[](size_t s) {
    auto x = memory::KMemory::allocImpl(s);
    if (x)
    {
        return !x;
    }
    else
    {
        // Trigger an exception
        __asm__ __volatile__("int $24;" : : );
        return NULL;
    }
}

void operator delete(void *p) {
    memory::KMemory::freeImpl(p);
}

void operator delete[](void *p) {
    memory::KMemory::freeImpl(p);
}

void operator delete(void *p, size_t sz) {
    if (!memory::KMemory::freeImpl(p, sz))
    {
        // Trigger an exception
        __asm__ __volatile__("int $25;" : : );
    }
}

void operator delete[](void *p, size_t sz) {
    if (!memory::KMemory::freeImpl(p, sz))
    {
        // Trigger an exception
        __asm__ __volatile__("int $25;" : : );
    }
}

