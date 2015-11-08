#ifndef _KLIBS_PALLOC_H
#define _KLIBS_PALLOC_H

#include <stdint.h>
#include <stddef.h>

namespace palloc
{
    class Page
    {
    public:
        Page(uint32_t phys_addr, uint32_t flags);
        uint32_t phys_addr;
        uint32_t flags;
    };

    class Page4M : Page {};
    class Page4K : Page {};

    class CommonPageManager
    {
    public:
        CommonPageManager();
    };
}

#endif /* _KLIBS_PALLOC_H */
