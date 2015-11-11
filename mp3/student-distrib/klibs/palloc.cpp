#include <inc/klibs/palloc.h>

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

    MemMap::MemMap()
    {
        clear();
    }

    inline void MemMap::clear()
    {
        memset(phys2virt, 0, sizeof(phys2virt));
        memset(virt2phys, 0, sizeof(virt2phys));

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
    void MemMap::loadToCR3(spinlock_t* cpuPagingLock)
    {
        uint32_t flags;

        // clear the NULL page.
        virt2phys[0] = 0;

        // load in the kernel code page.
        phys2virt[1] = 1;
        virt2phys[1] = PhysAddr(1, 0).pde;

        spin_lock_irqsave(cpuPagingLock, flags);
        REDIRECT_PAGE_DIR(virt2phys, 0);
        RELOAD_CR3();
        spin_unlock_irqrestore(cpuPagingLock, flags);
    }

}
