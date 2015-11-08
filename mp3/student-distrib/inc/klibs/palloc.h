#ifndef _KLIBS_PALLOC_H
#define _KLIBS_PALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <inc/bitset.h>
#include <inc/stack.h>
#include <inc/maybe.h>

namespace palloc
{
    static constexpr size_t KB = 1024;
    static constexpr size_t MB = 1024 * 1024;

    typedef struct
    {
        // This is just the PD INDEX part.
        uint16_t phys_addr;
        uint16_t flags;
    } Page4M;

    // MaxMemory must be divisible by 4MB
    template <size_t MaxMemory>
    class PhysPageManager
    {
    private:
        static constexpr size_t SmallestPageSize = 4 * MB;
        static constexpr size_t MaxNumElements = MaxMemory / SmallestPageSize;
        util::BitSet<MaxNumElements> isPhysAddrUsed;
        util::Stack<uint16_t, MaxNumElements> freePhysAddr;
    public:
        util::BitSet<MaxNumElements> isCommonPage;

        PhysPageManager();

        // return 0xffff <=> Memory is Full.
        uint16_t allocPage(int8_t isCommonPage);

        // if this page is NOT in use, do nothing.
        void freePage(uint16_t pageIndex);       
    };

    PhysPageManager<0xff000000> physPages;

    // MaxSize and startAddr must be divisible by 4MB
    //   It allocates one 4MB virtual mem at a time.
    template <size_t startAddr, size_t MaxSize>
    class VirtualMemRegion
    {
    private:
        static constexpr size_t SmallestPageSize = 4 * MB;
        static constexpr size_t MaxNumElements = MaxSize / SmallestPageSize;
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
}

#endif /* _KLIBS_PALLOC_H */
