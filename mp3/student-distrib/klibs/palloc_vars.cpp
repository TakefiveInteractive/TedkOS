#include <inc/klibs/palloc.h>
#include <inc/mbi_info.h>

namespace palloc {

// Assume the machine has at most 4080 MB memory.
//  Also: the manager will scan the actual memory map provided by multiboot info
//          to determine which parts of memory actually exist.
PhysPageManager<0xff000000> physPages(MultiBootInfoAddress);
// 0xc0000000 - 0x3fc00000 is the Last 1GB of virtual address space
VirtualMemRegion<0xc0000000, 0x3fc00000> virtLast1G;
MemMapManager cpu0_memmap(&cpu0_paging_lock);

}

