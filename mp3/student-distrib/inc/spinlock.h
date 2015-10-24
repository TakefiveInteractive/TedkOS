#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#define SPINLOCK_LOCKED 1
#define SPINLOCK_UNLOCKED 0

#include <stdint.h>
#include <stddef.h>

typedef uint32_t spinlock_t;

extern void spin_lock_init(spinlock_t* lock);
extern void spin_unlock(spinlock_t* lock);
extern void spin_lock(spinlock_t* lock);
extern uint32_t spin_trylock(spinlock_t* lock);
extern uint32_t spin_is_locked(spinlock_t* lock);

#define spin_lock_irqsave(lock, flags) {    \
    asm volatile (                          \
        "pushfl                      ;"     \
        "popl %0                     ;"     \
        "cli                         ;"     \
        : "=rm" ((flags)) : : "cc");        \
    spin_lock((lock));                      \
}

#define spin_unlock_irqrestore(lock, flags) {   \
    spin_unlock((lock));                        \
    asm volatile (                              \
        "pushl %0                    ;"         \
        "popfl                       ;"         \
        : : "rm" ((flags)) : "cc");             \
}

#endif // _SPINLOCK_H_
