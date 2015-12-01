#ifndef _KLIBS_PALLOC_H
#define _KLIBS_PALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/bitset.h>
#include <inc/klibs/stack.h>
#include <inc/klibs/maybe.h>
#include <inc/mbi_info.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/spinlock.h>
#include <inc/x86/paging.h>
#include <inc/klibs/memory.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/maybe.h>
#include <inc/x86/err_handler.h>

using memory::operator "" _KB;
using memory::operator "" _MB;

namespace palloc
{
    static constexpr uint32_t PhysIdxLength = 12;
    // We try to use 12 + 22 bit address (4MB page)
    // thus there are 1 << 12 possible page indices.
    // and we stores index of a page using uint16_t

    class VirtAddr
    {
    public:
        VirtAddr(void* _addr);
        void* addr;
    };

    class PhysAddr
    {
    public:
        PhysAddr(uint32_t _pde);
        PhysAddr(uint16_t pageIndex, uint32_t flags);
        uint16_t index() const;
        uint32_t flags() const;
        uint32_t pde;

        bool operator == (const PhysAddr& other) const
        {
            // CPU may touch the access or dirty bit
            return (pde & PG_NOT_ACCESSED & PG_NOT_DIRTY) == (other.pde & PG_NOT_ACCESSED & PG_NOT_DIRTY);
        }

        bool operator != (const PhysAddr& other) const
        {
            return !(*this == other);
        }

    };

    // MaxMemory must be divisible by 4MB
    template <uint32_t MaxMemory>
    class PhysPageManager
    {
    private:
        static constexpr uint32_t SmallestPageSize = 4_MB;
        static constexpr uint32_t MaxNumElements = MaxMemory / SmallestPageSize;
        util::BitSet<MaxNumElements> isPhysAddrFree;
        util::Stack<uint16_t, MaxNumElements> freePhysAddr;
        spinlock_t lock = SPINLOCK_UNLOCKED;
    public:
        util::BitSet<MaxNumElements> isCommonPage;

        PhysPageManager(multiboot_info_t* mbi);

        // Fails when memory is Full.
        Maybe<uint16_t> allocPage(bool isCommonPage);

        // if this page is NOT in use, do nothing.
        void freePage(uint16_t pageIndex);

        void freePage(PhysAddr page);
    };

    // MaxSize and startAddr must be divisible by 4MB
    //   It allocates one 4MB virtual mem at a time.
    template <uint32_t startAddr, uint32_t MaxSize>
    class VirtualMemRegion
    {
    private:
        static constexpr uint32_t SmallestPageSize = 4_MB;
        static constexpr uint32_t MaxNumElements = MaxSize / SmallestPageSize;
        util::BitSet<MaxNumElements> isVirtAddrUsed;
        util::Stack<uint16_t, MaxNumElements> freeVirtAddr;
        spinlock_t lock = SPINLOCK_UNLOCKED;
    public:
        util::BitSet<MaxNumElements> isCommonPage;

        VirtualMemRegion();

        // Fails when memory is full
        Maybe<void*> allocPage(bool isCommonPage);

        Maybe<void*> allocConsPage(size_t num, bool isCommonPage);

        // if this page is NOT in use, do nothing.
        void freePage(void* virtAddr);
    };

    // 4MB page only. This manages a set of page mappings as an object.
    // !!!! The only two things that are hard coded are:
    //          1. 0~4 MB page is NOT PRESENT
    //          2. 4~8 MB page is kernel code. readonly & privileged.
    // THUS: adding any mapping below is NOT allowed:
    //          1. virt index = 0 or 1
    // WE DO NOT ALLOW:
    //      multi to one, one to multi, or multi to multi mapping.
    // DO NOT USE THIS DURING entry() in kernel.cpp !!!
    //      THIS is for user & kernel processes to use.
    //      THAT requires a more special mapping.
    class MemMap
    {
        friend class MemMapManager;
    private:
        // arrays to translate indices between phys space and virt space.
        //  Some thing is special about virt2phys:
        //     results already contain flags (can function as page dir)
        // For both arrays, a 0 entry means "not exist".
        uint16_t phys2virt[1 << PhysIdxLength];
        volatile uint32_t virt2phys[PD_NUM_ENTRIES] __attribute__((aligned (4096)));
        inline void clear();
    public:
        // Initialize an empty memory map;
        MemMap();

        // Translating is only based on INDEX, NOT flags
        inline VirtAddr translate(const PhysAddr& addr);
        inline PhysAddr translate(const VirtAddr& addr);

        // Add an entry of mapping to this map.
        // WARNING: If an entry already exists at that location,
        //      This will return false. Otherwise returns true.
        bool add(const VirtAddr& virt, const PhysAddr& phys);

        // Remove an entry of mapping.
        // WARNING: If an entry DID NOT exist at that location,
        //      OR IF THAT PHYS ADDR is RESERVED for GLOBAL KERNEL.
        //      This will return false. Otherwise returns true.
        bool operator -= (const VirtAddr& addr);
        bool operator -= (const PhysAddr& addr);

        // Add this map with another map. Return true if successful.
        //   !! Return false if conflict occurs !!
        // We START FROM 2, to ignore Kernel at 4MB ~ 8MB
        //              and 0MB ~ 4MB is NOT PRESENT !!!!
        //                  (UNLIKE kernel init stage where vmem -> somewhere 1MB)
        bool operator += (const MemMap& that);

        void operator = (const MemMap& other)
        {
            memcpy(&phys2virt, &other.phys2virt, sizeof(phys2virt));
            memcpy((void *)&virt2phys, (void *)&other.virt2phys, sizeof(virt2phys));
        }

        // Currently we have only one cpu. Thus cpuPagingLock -> cpu0_paging_lock
        //  THIS WILL flush TLB
        // !!!!!!! CALL THIS ONLY if the variable is on 4MB~8MB Page !!!!!!!!!
        //      ( so that its virtual address = physical address )
        inline void loadToCR3();

        inline bool isLoadedToCR3();
    };

    class TinyMemMap
    {
        friend class MemMapManager;
    private:
        class Mapping
        {
        public:
            // Use the default constructor will fill values with INVALID address
            Mapping();
            Mapping(const PhysAddr& p, const VirtAddr& v);
            PhysAddr phys;
            VirtAddr virt;
        };
        util::Stack<TinyMemMap::Mapping, 1_KB> pdStack;
        util::BitSet<1_KB> isVirtAddrUsed;
    public:
        bool add(const VirtAddr& virt, const PhysAddr& phys);
        bool remove(const VirtAddr& virt, const PhysAddr& physIdx);
    };

    // This mamanger manages a whole cpu's process map and common map
    //  common map is completely private, not overwritable by others.
    //  CR3 is automatically reload unless otherwise specified.
    //
    // By default the manager is NOT in service. You need to call start()
    class MemMapManager
    {
    private:
        MemMap spareMemMaps[2];
        int loadedMap;
        // If false, we are using the static plain old Page Dir
        // If true, this manager is in charge.
        bool bIsStarted;
        MemMap commonMemMap;
        spinlock_t* cpu_cr3_lock;
    public:
        MemMapManager(spinlock_t* cpu_cr3_lock);

        // Translating is only based on INDEX, NOT flags
        VirtAddr translate(const PhysAddr& addr);
        PhysAddr translate(const VirtAddr& addr);

        bool addCommonPage(const VirtAddr& virt, const PhysAddr& phys);

        // delete the mapping ONLY.
        // to RELEASE the ACTUAL SPACE, call Phys/VirtAddrManager.freePage
        bool delCommonPage(const VirtAddr& virt);
        bool delCommonPage(const PhysAddr& phys);
        bool loadProcessMap(const TinyMemMap& map);

        bool isStarted();

        // Start service, and DICARD the old static map in kernel
        // This changes a lot of things, for example:
        //   You need to update video memory address.
        void start();

        // Stop service and change back to the old static map in kernel
        void stop(uint32_t* pageDir);
    };

    // This function tries to find or create a virtual address for page 0MB - 4MB
    // If it returns 0xffffffff, then memory is used up.
    Maybe<uint32_t> virtOfPage0();

    // Assume the machine has at most 4080 MB memory.
    //  Also: the manager will scan the actual memory map provided by multiboot info
    //          to determine which parts of memory actually exist.
    extern PhysPageManager<0xff000000> physPages;

    // 0xc0000000 - 0xffc00000 is the Last 1GB of virtual address space
    extern VirtualMemRegion<0xc0000000, 0x3fc00000> virtLast1G;

    extern MemMapManager cpu0_memmap;

    // 2.5G - 3G is allocated for HD VMem
    // 0xa0000000 - 0xc0000000
    static constexpr uint32_t virtForHDVmem = 0xa0000000;
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
                (uint32_t) mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *) ((uint32_t) mmap
                    + mmap->size + sizeof (mmap->size)))
        {
            uint32_t start_low, start_high, end_low, end_high;
            uint32_t start_idx, end_idx;
            // [start, end)
            start_low = mmap->base_addr_low;
            start_high = mmap->base_addr_high;

            // Avoid memory larger than 4GB
            if(start_high > 0)
                continue;

            if(mmap->length_high > (0xffffffff - start_high))
                continue;
            end_high = start_high + mmap->length_high;
            if(mmap->length_low > (0xffffffff - start_low))
                end_high++;
            end_low = start_low + mmap->length_low;

            printf("start = %x %x\n", start_high, start_low);

            start_idx = (start_low >> 22) | (start_high << 10);

            // Avoid addition overflow.
            if(((1 - (1 << 10)) << 22) & end_high)
                continue;

            // Avoid memory larger than 4GB
            if(end_high == 0)
                end_idx = (end_low >> 22) | (end_high << 10);
            else end_idx = 0xffc;

            // This is an available memory.
            if(mmap->type == 1)
            {
                for(uint32_t idx = start_idx; idx < end_idx; idx++) if(idx != 0 && idx != 1)
                {
                    if(!isPhysAddrFree.test(idx))
                    {
                        isPhysAddrFree.set(idx);
                        freePhysAddr.push(idx);
                    }
                }
            }
        }
    }
    else
    {
        // 0 and 1 are pages used by kernel initialization
        for(uint16_t i = MaxNumElements - 1; i >= 2; i++)
            freePhysAddr.push(i);
    }

    // 0 and 1 are pages used by kernel initialization
    isPhysAddrFree.clear(0);
    isPhysAddrFree.clear(1);

    // go through modules and mark them as occupied
    size_t modCount = mbi->mods_count;
    module_t* mod = (module_t*) mbi->mods_addr;
    for (size_t i = 0; i < modCount; i++)
    {
        uint32_t start = mod[i].mod_start;
        uint32_t end = mod[i].mod_end;
        uint32_t numPages = memory::ceil((end - start), 4_MB);
        for (size_t j = 0; j < numPages; j++)
        {
            isPhysAddrFree.clear((start + j * 4_MB) >> 22);
        }
    }
}

template <uint32_t MaxMemory>
Maybe<uint16_t> PhysPageManager<MaxMemory>::allocPage(bool isCommonPage)
{
    AutoSpinLock l(&lock);
    if(freePhysAddr.empty()) return Maybe<uint16_t>();
    uint16_t top = freePhysAddr.pop();
    if(!isPhysAddrFree.test(top)) return Maybe<uint16_t>();
    isPhysAddrFree.clear(top);
    if(isCommonPage)
        this->isCommonPage.set(top);
    return Maybe<uint16_t>(top);
}

template <uint32_t MaxMemory>
void PhysPageManager<MaxMemory>::freePage(uint16_t pageIndex)
{
    AutoSpinLock l(&lock);
    if(isPhysAddrFree.test(pageIndex)) return;
    isPhysAddrFree.set(pageIndex);
    isCommonPage.clear(pageIndex);
    freePhysAddr.push(pageIndex);
}

template <uint32_t MaxMemory>
void PhysPageManager<MaxMemory>::freePage(PhysAddr page)
{
    freePage(page.index());
}

template <uint32_t startAddr, uint32_t MaxSize>
VirtualMemRegion<startAddr, MaxSize>::VirtualMemRegion()
{
    for (uint32_t i = 0; i < MaxNumElements; i++)
        freeVirtAddr.push(i);
}

template <uint32_t startAddr, uint32_t MaxSize>
Maybe<void*> VirtualMemRegion<startAddr, MaxSize>::allocConsPage(size_t num, bool isCommonPage)
{
    AutoSpinLock l(&lock);
    if (freeVirtAddr.empty()) return Maybe<void*>();

    // Super dirty algorithm here. To be improved.
    Maybe<size_t> _startIdx = isVirtAddrUsed.findConsZeros(num);
    if (_startIdx)
    {
        size_t startIdx = +_startIdx;
        // Kill all corresponding items in the stack
        for (size_t i = startIdx; i < startIdx + num; i++)
        {
            size_t idxInStack = 0;
            auto good = freeVirtAddr.template first<size_t>(idxInStack, [i](auto x) {
                if (i == x)
                    return Maybe<size_t>(0);
                else
                    return Maybe<size_t>();
            });
            if (!good) trigger_exception<27>();
            freeVirtAddr.drop(idxInStack);
            if (isCommonPage) this->isCommonPage.set(i);
        }
        return Maybe<void*>((void*)(startIdx * 4_MB + startAddr));
    }
    else
    {
        return Maybe<void*>();
    }
}

template <uint32_t startAddr, uint32_t MaxSize>
Maybe<void*> VirtualMemRegion<startAddr, MaxSize>::allocPage(bool isCommonPage)
{
    AutoSpinLock l(&lock);
    if (freeVirtAddr.empty()) return Maybe<void*>();

    uint16_t top = freeVirtAddr.pop();
    isVirtAddrUsed.set(top);
    if (isCommonPage) this->isCommonPage.set(top);

    return Maybe<void*>((void*)(top * 4_MB + startAddr));
}

template <uint32_t startAddr, uint32_t MaxSize>
void VirtualMemRegion<startAddr,MaxSize>::freePage(void* virtAddr)
{
    AutoSpinLock l(&lock);
    uint16_t pageIndex = ((uint32_t)virtAddr >> 22);
    if (isVirtAddrUsed.test(pageIndex))
    {
        isVirtAddrUsed.clear(pageIndex);
        isCommonPage.clear(pageIndex);
        freeVirtAddr.push(pageIndex);
    }
}

}

#endif /* _KLIBS_PALLOC_H */
