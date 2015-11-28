#ifndef _INIT_H
#define _INIT_H

#include <stdint.h>
#include <stddef.h>

extern uint32_t userFirst4MBTable[1024];

extern "C"
{
    void init_main();
}

#endif /* _INIT_H */
