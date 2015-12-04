#ifndef _INIT_H
#define _INIT_H

#include <stddef.h>
#include <stdint.h>

// If false: no process exists yet, indicating that calling getCurrentThreadInfo() will fail.
// Otherwise: getCurrentThreadInfo() will work fine.
extern volatile bool pcbLoadable;
extern volatile bool canUseCpp;
extern volatile bool isFallbackTerm;
extern uint32_t userFirst4MBTable[1024];

extern "C"
{
    void init_main(void* arg);
}

#endif /* _INIT_H */
