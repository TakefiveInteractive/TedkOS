#include <inc/klibs/palloc.h>
#include <inc/mbi_info.h>

namespace palloc {

PhysPageManager<0xff000000> physPages(MultiBootInfoAddress);
VirtualMemRegion<0xc0000000, 0x3fc00000> virtLast1G;
MemMap currProcMemMap;
MemMap commonMemMap;

}

