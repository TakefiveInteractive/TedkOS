#ifndef _INIT_H
#define _INIT_H

// If false: no process exists yet, indicating that calling getCurrentThreadInfo() will fail.
// Otherwise: getCurrentThreadInfo() will work fine.
extern volatile bool pcbLoadable;
extern volatile bool canUseCpp;

extern "C"
{
    void init_main();
}

#endif /* _INIT_H */
