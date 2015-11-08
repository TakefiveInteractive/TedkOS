#include <inc/klibs/palloc.h>

namespace palloc {

PhysPageManager<0xff000000> physPages;
VirtualMemRegion<0xc0000000, 0x3fc00000> virtLast1G;

}

