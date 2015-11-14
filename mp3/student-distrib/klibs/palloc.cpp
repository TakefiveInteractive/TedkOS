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

    inline uint16_t PhysAddr::index() const
    {
        return ((pde & 0xffc00000) >> 22) | ((pde & 0x00006000) >> 3);
    }

    inline uint32_t PhysAddr::flags() const
    {
        return pde & 0x1fff;
    }

    MemMap::MemMap() : phys2virt(), virt2phys()
    {
        clear();
    }

    inline void MemMap::clear()
    {
        // load in the kernel code page.
        phys2virt[1] = 1;
        virt2phys[1] = PhysAddr(1, 0).pde;
    }

    inline VirtAddr MemMap::translate(const PhysAddr& addr)
    {
        return VirtAddr((void*)(phys2virt[addr.index()] << 22));
    }

    inline PhysAddr MemMap::translate(const VirtAddr& addr)
    {
        return PhysAddr(virt2phys[((uint32_t)addr.addr)>>22]);
    }

    // Add an entry of mapping to this map.
    // WARNING: If an entry already exists at that location,
    //      This will return false. Otherwise returns true.
    bool MemMap::add(const VirtAddr& virt, const PhysAddr& phys)
    {
        uint16_t vidx = ((uint32_t)virt.addr) >> 22;
        if((virt2phys[vidx] & PAGING_PRESENT) || phys2virt[phys.index()] != 0)
            return false;

        if(vidx == 0 || vidx == 1)
            return false;

        virt2phys[     vidx     ] = phys.pde;
        phys2virt[ phys.index() ] = vidx;
        return true;
    }

    // Remove an entry of mapping.
    // WARNING: If an entry DID NOT exist at that location,
    //      OR IF THAT PHYS ADDR is RESERVED for GLOBAL KERNEL.
    //      This will return false. Otherwise returns true.
    bool MemMap::operator -= (const VirtAddr& addr)
    {
        uint16_t vidx = ((uint32_t)addr.addr) >> 22;
        if(!(virt2phys[vidx] & PAGING_PRESENT))
            return false;
        uint16_t pidx = PhysAddr(virt2phys[vidx]).index();

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
        if(isVirtAddrUsed.test(((uint32_t)virt.addr)>>22))
            return false;
        isVirtAddrUsed.set(((uint32_t)virt.addr)>>22);
        TinyMemMap::Mapping map(phys, virt);
        pdStack.push(map);
        return true;
    }

    TinyMemMap::Mapping::Mapping(const PhysAddr& p, const VirtAddr& v)
        : phys (p), virt(v) {}
    
    MemMapManager::MemMapManager(spinlock_t* cpu_cr3_lock)
    {
        loadedMap = 0;
        isStarted = false;
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
        if(!isStarted)
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
        if(!isStarted)
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
        if(!isStarted)
            global_cr3val[((uint32_t)virt.addr) >> 22] = 0;
        RELOAD_CR3();
        return true;
    }

    bool MemMapManager::loadProcessMap(const TinyMemMap& map)
    {
        if(!isStarted)
            return false;
        AutoSpinLock lock(cpu_cr3_lock);
        for(uint16_t i = 2; i < PD_NUM_ENTRIES; i++)
        {
            bool ours   = commonMemMap.virt2phys[i] & PAGING_PRESENT;
            bool theirs = map.isVirtAddrUsed.test(i);
            if(ours && theirs)
                return false;
        }
        spareMemMaps[1 - loadedMap] = MemMap();
        map.pdStack.first((Maybe<bool> (*)(TinyMemMap::Mapping, MemMapManager*))[](auto val, auto _this)
        {
            _this->spareMemMaps[1 - _this->loadedMap].add(val.virt, val.phys);
            return Maybe<bool>(true);
        }, this);
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
        isStarted = true;
    }

    // Stop service and change back to the old static map in kernel
    void MemMapManager::stop(uint32_t* pageDir)
    {
        AutoSpinLock lock(cpu_cr3_lock);
        REDIRECT_PAGE_DIR(pageDir);
        RELOAD_CR3();
        isStarted = false;
    }
}
