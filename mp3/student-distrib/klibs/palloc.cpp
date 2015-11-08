#include <inc/klibs/palloc.h>

using namespace palloc;

Page::Page(uint32_t phys_addr, uint32_t flags)
{
    this->phys_addr = phys_addr;
    this->flags     = flags;
}
