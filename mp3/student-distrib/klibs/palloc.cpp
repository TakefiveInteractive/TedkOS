#include <inc/klibs/palloc.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/x86/paging.h>

namespace palloc
{
    VirtAddr::VirtAddr(void* _addr)
    {
        addr = _addr;
    }

    PhysAddr::PhysAddr(uint32_t _pde)
    {
        pde  = _pde;
    }

    PhysAddr::PhysAddr(uint16_t pageIndex, uint32_t flags)
    {
        // Here we allow as much as 16GB physical memory.
        pde  = PG_4MB_BASE | ((pageIndex & 0x3ff) << 22) | ((pageIndex & 0xc00) << 3) | flags;
    }

    uint16_t PhysAddr::index() const
    {
        // & 0xffc00000 >> 2 extracts the LOWEST 10 bits of the index of a 4MB page from PDE
        // pde & 0x6000 extracts the HIGHER 2 bits of the index of a 4MB page from PDE
        //      THUS: this operation extracts the index as a whole
        return ((pde & 0xffc00000) >> 22) | ((pde & 0x00006000) >> 3);
    }

    uint32_t PhysAddr::flags() const
    {
        // all flags lie in the lower 13 bits of PDE
        //  all information about index lies in higher 19 bits
        //  Thus & 0x1fff extracts only the flags
        return pde & 0x1fff;
    }

    MemMap::MemMap() : phys2virt(), virt2phys()
    {
        clear();
    }

    inline void MemMap::clear()
    {
        memset(phys2virt, 0, sizeof(phys2virt));
        memset((void*)virt2phys, 0, sizeof(virt2phys));
        // load in the kernel code page.
        // That is, map 4MB ~ 8MB (index 1) in virtual memory to 4MB ~ 8MB in Physical memory
        //   We do not do the same thing for 0MB~4MB, because it is not needed,
        //      and because if we do that it will cause *NULL to run without exception.
        phys2virt[1] = 1;
        virt2phys[1] = PhysAddr(1, 0).pde;
    }

    inline VirtAddr MemMap::translate(const PhysAddr& addr)
    {
        // << 22 turns index to the address of the 4MB (2^22) page
        return VirtAddr((void*)(phys2virt[addr.index()] << 22));
    }

    inline PhysAddr MemMap::translate(const VirtAddr& addr)
    {
        // >> 22 turns address of 4MB (2^22) page to the index
        return PhysAddr(virt2phys[((uint32_t)addr.addr) >> 22]);
    }

    // Add an entry of mapping to this map.
    // WARNING: If an entry already exists at that location,
    //      This will return false. Otherwise returns true.
    bool MemMap::add(const VirtAddr& virt, const PhysAddr& phys)
    {
        // >> 22 turns address of 4MB (2^22) page to the index
        uint16_t vidx = ((uint32_t)virt.addr) >> 22;

        if(phys.index() != 1 // Special handling for double-mapping the kernel page
            && ((virt2phys[vidx] & PAGING_PRESENT) || phys2virt[phys.index()] != 0))
            return false;

        if(vidx == 0 || vidx == 1)
            return false;

        if (phys.index() != 1) phys2virt[phys.index()] = vidx;
        virt2phys[vidx] = phys.pde;
        return true;
    }

    // Remove an entry of mapping.
    // WARNING: If an entry DID NOT exist at that location,
    //      OR IF THAT PHYS ADDR is RESERVED for GLOBAL KERNEL.
    //      This will return false. Otherwise returns true.
    bool MemMap::operator -= (const VirtAddr& addr)
    {
        // >> 22 turns address of 4MB (2^22) page to the index
        uint16_t vidx = ((uint32_t)addr.addr) >> 22;
        if(!(virt2phys[vidx] & PAGING_PRESENT))
            return false;
        uint16_t pidx = PhysAddr(virt2phys[vidx]).index();

        // We ban mappings with virtual index = 0, they will enable dereferencing NULL
        // We ban mappings with virtual index = 1, except for the ones we hard coded in
        //      constructor. Otherwise kmalloc may use space where kernel code resides.
        if(vidx == 0 || vidx == 1)
            return false;

        virt2phys[vidx] = 0;
        phys2virt[pidx] = 0;
        return true;
    }

    bool MemMap::operator -= (const PhysAddr& addr)
    {
        uint16_t pidx = addr.index();
        if(phys2virt[pidx] == 0)
            return false;
        uint16_t vidx = phys2virt[pidx];

        // We ban mappings with virtual index = 0, they will enable dereferencing NULL
        // We ban mappings with virtual index = 1, except for the ones we hard coded in
        //      constructor. Otherwise kmalloc may use space where kernel code resides.
        if(vidx == 0 || vidx == 1)
            return false;

        virt2phys[vidx] = 0;
        phys2virt[pidx] = 0;
        return true;
    }


    // Add this map with another map. Return a new map.
    //   !! Return NULL if conflict occurs !!
    bool MemMap::operator += (const MemMap& that)
    {
        // We START FROM 2, to ignore Kernel at 4MB ~ 8MB
        //              and 0MB ~ 4MB is NOT PRESENT
        //                  (UNLIKE kernel init stage where vmem -> somewhere 1MB)
        // iterate on virtual address index
        for(uint16_t i = 2; i < PD_NUM_ENTRIES; i++)
        {
            bool ours   = virt2phys[i] & PAGING_PRESENT;
            bool theirs = that.virt2phys[i] & PAGING_PRESENT;
            if(ours && theirs)
                return false;
        }

        // iterate on virtual address index
        for(uint16_t i = 2; i < PD_NUM_ENTRIES; i++)
        {
            if(that.virt2phys[i] & PAGING_PRESENT)
            {
                uint16_t pidx = PhysAddr(that.virt2phys[i]).index();
                virt2phys[i] = that.virt2phys[i];
                phys2virt[pidx] = that.phys2virt[pidx];
            }
        }

        return true;
    }

    // Currently we have only one cpu. Thus cpuPagingLock -> cpu0_paging_lock
    //  THIS WILL flush TLB
    // !!!!!!! CALL THIS ONLY if the variable is on 4MB~8MB Page !!!!!!!!!
    //      ( so that its virtual address = physical address )
    inline void MemMap::loadToCR3()
    {
        // clear the NULL page.
        virt2phys[0] = 0;

        // load in the kernel code page.
        phys2virt[1] = 1;
        virt2phys[1] = PhysAddr(1, 0).pde;

        REDIRECT_PAGE_DIR(virt2phys);
        RELOAD_CR3();
    }

    inline bool MemMap::isLoadedToCR3()
    {
        return ((uint32_t)global_cr3val & ALIGN_4KB_ADDR) == (uint32_t) virt2phys;
    }

    bool TinyMemMap::add(const VirtAddr& virt, const PhysAddr& phys)
    {
        // >> 22 turns address of 4MB (2^22) page to the index
        auto idx = ((uint32_t) virt.addr) >> 22;
        if (isVirtAddrUsed.test(idx))
            return false;
        isVirtAddrUsed.set(idx);
        TinyMemMap::Mapping map(phys, virt);
        pdStack.push(map);
        return true;
    }

    bool TinyMemMap::remove(const VirtAddr& virt, const PhysAddr& physIdx)
    {
        auto idx = ((uint32_t) virt.addr) >> 22;
        if (isVirtAddrUsed.test(idx))
        {
            isVirtAddrUsed.clear(idx);
            // Kill the corresponding entry from stack
            return pdStack.dropFirst([&physIdx](auto mapping) { return mapping.phys == physIdx; });
         }
        return false;
    }

    TinyMemMap::Mapping::Mapping(const PhysAddr& p, const VirtAddr& v)
        : phys (p), virt(v) {}

    // physical address with index = 0xffff (our pde only uses 12 bits),
    //  AND virtual address with addr = NULL ARE INVALID.
    // we use the invalid values to initialize an entry of mapping,
    //      unless user specifies more information (using the method above)
    TinyMemMap::Mapping::Mapping()
        : phys (0xffff, 0), virt(NULL) {}

    MemMapManager::MemMapManager(spinlock_t* cpu_cr3_lock)
    {
        loadedMap = 0;
        bIsStarted = false;
        this->cpu_cr3_lock = cpu_cr3_lock;
    }

    bool MemMapManager::addCommonPage(const VirtAddr& virt, const PhysAddr& phys)
    {
        AutoSpinLock lock(cpu_cr3_lock);
        if(!commonMemMap.add(virt, phys))
            return false;
        if(!spareMemMaps[loadedMap].add(virt, phys))
        {
            commonMemMap -= virt;
            return false;
        }

        // >> 22 turns address of 4MB (2^22) page to the index
        if(!bIsStarted)
            global_cr3val[((uint32_t)virt.addr) >> 22] = phys.pde;
        RELOAD_CR3();
        return true;
    }

    bool MemMapManager::delCommonPage(const VirtAddr& virt)
    {
        AutoSpinLock lock(cpu_cr3_lock);
        PhysAddr phys = commonMemMap.translate(virt);
        if(!(commonMemMap -= virt))
            return false;
        if(!(spareMemMaps[loadedMap] -= virt))
        {
            commonMemMap.add(virt, phys);
            return false;
        }
        // >> 22 turns address of 4MB (2^22) page to the index
        if(!bIsStarted)
            global_cr3val[((uint32_t)virt.addr) >> 22] = 0;
        RELOAD_CR3();
        return true;
    }

    bool MemMapManager::delCommonPage(const PhysAddr& phys)
    {
        AutoSpinLock lock(cpu_cr3_lock);
        VirtAddr virt = commonMemMap.translate(phys);
        if(!(commonMemMap -= virt))
            return false;
        if(!(spareMemMaps[loadedMap] -= virt))
        {
            commonMemMap.add(virt, phys);
            return false;
        }
        // >> 22 turns address of 4MB (2^22) page to the index
        if(!bIsStarted)
            global_cr3val[((uint32_t)virt.addr) >> 22] = 0;
        RELOAD_CR3();
        return true;
    }

    bool MemMapManager::loadProcessMap(const TinyMemMap& map)
    {
        if(!bIsStarted)
            return false;
        AutoSpinLock lock(cpu_cr3_lock);
        // We START FROM 2, to ignore Kernel at 4MB ~ 8MB
        //              and 0MB ~ 4MB is NOT PRESENT
        //                  (UNLIKE kernel init stage where vmem -> somewhere 1MB)
        spareMemMaps[1 - loadedMap].clear();
        auto conflict = map.pdStack.template first<int>([this, &map](auto val) -> Maybe<int>
        {
            uint16_t virtIdx = ((uint32_t) val.virt.addr) >> 22;
            bool ours = commonMemMap.virt2phys[virtIdx] & PAGING_PRESENT;
            bool theirs = map.isVirtAddrUsed.test(virtIdx);
            // Skip kernel pages
            if (virtIdx >= 2 && ours && theirs
            && commonMemMap.translate(val.virt) != val.phys)
                return 0;
            return Nothing;
        });
        if (conflict) return false;

        map.pdStack.template first<bool>([this](auto val)
        {
            spareMemMaps[1 - loadedMap].add(val.virt, val.phys);
            return Nothing;
        });

        spareMemMaps[1 - loadedMap] += commonMemMap;
        spareMemMaps[1 - loadedMap].loadToCR3();
        loadedMap = 1 - loadedMap;
        return true;
    }

    // Start service, and DICARD the old static map in kernel
    // This changes a lot of things, for example:
    //   You need to update video memory address.
    void MemMapManager::start()
    {
        AutoSpinLock lock(cpu_cr3_lock);
        spareMemMaps[loadedMap].loadToCR3();
        bIsStarted = true;
    }

    // Stop service and change back to the old static map in kernel
    void MemMapManager::stop(uint32_t* pageDir)
    {
        AutoSpinLock lock(cpu_cr3_lock);
        REDIRECT_PAGE_DIR(pageDir);
        RELOAD_CR3();
        bIsStarted = false;
    }

    bool isStarted()
    {
        return bIsStarted;
    }

    VirtAddr MemMapManager::translate(const PhysAddr& addr)
    {
        // << 22 turns index to the address of the 4MB (2^22) page
        return spareMemMaps[loadedMap].translate(addr);
    }

    PhysAddr MemMapManager::translate(const VirtAddr& addr)
    {
        // >> 22 turns address of 4MB (2^22) page to the index
        return spareMemMaps[loadedMap].translate(addr);
    }

    // This function tries to find or create a virtual address for page 0MB - 4MB
    // If it returns 0xffffffff, then memory is used up.
    Maybe<uint32_t> virtOfPage0()
    {
        if(!cpu0_memmap.isStarted())
            return Maybe<uint32_t>(0);

        // Search for a page from 0MB - 4MB (index=0)
        PhysAddr page0(0, PG_WRITABLE);

        VirtAddr base = cpu0_memmap.translate(page0);

        if (base.addr == NULL)
        {
            auto newAddr = virtLast1G.allocPage(true);
            if (!newAddr)
                return Nothing;
            base.addr = +newAddr;
            cpu0_memmap.addCommonPage(base, page0);
            return (uint32_t) base.addr;
        }
        return (uint32_t) base.addr;
    }
}
