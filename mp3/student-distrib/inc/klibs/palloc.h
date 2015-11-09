#ifndef _KLIBS_PALLOC_H
#define _KLIBS_PALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <inc/bitset.h>
#include <inc/stack.h>
#include <inc/maybe.h>
#include <inc/mbi_info.h>

namespace palloc
{
    static constexpr uint32_t KB = 1024;
    static constexpr uint32_t MB = 1024 * 1024;

    typedef struct
    {
        // This is just the PD INDEX part.
        uint16_t phys_addr;
        uint16_t flags;
    } Page4M;

    // MaxMemory must be divisible by 4MB
    template <uint32_t MaxMemory>
    class PhysPageManager
    {
    private:
        static constexpr uint32_t SmallestPageSize = 4 * MB;
        static constexpr uint32_t MaxNumElements = MaxMemory / SmallestPageSize;
        util::BitSet<MaxNumElements> isPhysAddrFree;
        util::Stack<uint16_t, MaxNumElements> freePhysAddr;
    public:
        util::BitSet<MaxNumElements> isCommonPage;

        PhysPageManager(multiboot_info_t* mbi);

        // return 0xffff <=> Memory is Full.
        uint16_t allocPage(int8_t isCommonPage);

        // if this page is NOT in use, do nothing.
        void freePage(uint16_t pageIndex);       
    };

    extern PhysPageManager<0xff000000> physPages;

    // MaxSize and startAddr must be divisible by 4MB
    //   It allocates one 4MB virtual mem at a time.
    template <uint32_t startAddr, uint32_t MaxSize>
    class VirtualMemRegion
    {
    private:
        static constexpr uint32_t SmallestPageSize = 4 * MB;
        static constexpr uint32_t MaxNumElements = MaxSize / SmallestPageSize;
        util::BitSet<MaxNumElements> isVirtAddrUsed;
        util::Stack<uint16_t, MaxNumElements> freeVirtAddr;
    public:
        util::BitSet<MaxNumElements> isCommonPage;

        VirtualMemRegion();

        // return 0xffff <=> Memory is Full.
        void* allocPage(int8_t isCommonPage);

        // if this page is NOT in use, do nothing.
        void freePage(void* virtAddr);       
    };

    extern VirtualMemRegion<0xc0000000, 0x3fc00000> virtLast1G;
}

//---------------------------------- IMPLEMENTATION -----------------------------------------

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

namespace palloc {

template <uint32_t MaxMemory>
PhysPageManager<MaxMemory>::PhysPageManager(multiboot_info_t* mbi)
{
    /* Are mmap_* valid? */
    if (CHECK_FLAG (mbi->flags, 6))
    {
        memory_map_t *mmap;

        for (mmap = (memory_map_t *) mbi->mmap_addr;
                (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *) ((unsigned long) mmap
                    + mmap->size + sizeof (mmap->size)))
        {
            uint32_t start_low, start_high, end_low, end_high;
            uint32_t start_idx, end_idx;
            // [start, end)
            start_low = mmap->base_addr_low;
            start_high = mmap->base_addr_high;
            if(mmap->length_high > (0xffffffff - start_high))
                continue;
            end_high = start_high + mmap->length_high;
            if(mmap->length_low > (0xffffffff - start_low))
                end_high++;
            end_low = start_low + mmap->length_low;

            if(((1 - (1<<10)) << 22) & start_high)
                continue;
            start_idx = (start_low >> 22) | (start_high << 10);

            if(((1 - (1<<10)) << 22) & end_high)
                continue;
            end_idx = (end_low >> 22) | (end_high << 10);

            // This is an available memory.
            if(mmap->type == 1)
            {
                for(uint32_t idx = start_idx; idx < end_idx; idx++)
                {
                    isPhysAddrFree.set(idx);
                    freePhysAddr.push(idx);
                }
            }
        }
    }
    else
    {
        // 0 and 1 are pages used by kernel initialization
        for(uint16_t i=MaxNumElements - 1; i >= 2; i++)
            freePhysAddr.push(i);
    }

    // 0 and 1 are pages used by kernel initialization
    isPhysAddrFree.clear(0);
    isPhysAddrFree.clear(1);
}

template <uint32_t MaxMemory>
uint16_t PhysPageManager<MaxMemory>::allocPage(int8_t isCommonPage)
{
    if(freePhysAddr.empty())
        return 0xffff;
    uint16_t top = freePhysAddr.pop();
    if(!isPhysAddrFree.test(top))
        return 0xffff;
    isPhysAddrFree.clear(top);
    if(isCommonPage)
        this->isCommonPage.set(top);
    return top;
}

template <uint32_t MaxMemory>
void PhysPageManager<MaxMemory>::freePage(uint16_t pageIndex)
{
    if(isPhysAddrFree.test(pageIndex))
        return;
    isPhysAddrFree.set(pageIndex);
    isCommonPage.clear(pageIndex);
    freePhysAddr.push(pageIndex);
    return;
}


template <uint32_t startAddr, uint32_t MaxSize>
VirtualMemRegion<startAddr,MaxSize>::VirtualMemRegion()
{
    for(uint32_t i = 0; i < MaxNumElements; i++)
        freeVirtAddr.push(i);
}

template <uint32_t startAddr, uint32_t MaxSize>
void* VirtualMemRegion<startAddr,MaxSize>::allocPage(int8_t isCommonPage)
{
    if(freeVirtAddr.empty())
        return (void*)0xffff;
    uint16_t top = freeVirtAddr.pop();
    isVirtAddrUsed.set(top);
    if(isCommonPage)
        this->isCommonPage.set(top);
    return (void*)(top << 22);
}

template <uint32_t startAddr, uint32_t MaxSize>
void VirtualMemRegion<startAddr,MaxSize>::freePage(void* virtAddr)
{
    uint16_t pageIndex = ((uint32_t)virtAddr >> 22);
    if(isVirtAddrUsed.test(pageIndex))
        return;
    isVirtAddrUsed.clear(pageIndex);
    isCommonPage.clear(pageIndex);
    freeVirtAddr.push(pageIndex);
    return;
}

}

#endif /* _KLIBS_PALLOC_H */
