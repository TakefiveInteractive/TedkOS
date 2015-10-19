#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "types.h"

#define SPINLOCK_LOCKED 1
#define SPINLOCK_UNLOCKED 0

typedef uint32_t spinlock_t;

extern void spin_lock_init(spinlock_t* lock);
extern void spin_lock_lock_irqsave(spinlock_t* lock, uint32_t flags);
extern void spin_lock_unlock_irqrestore(spinlock_t* lock, uint32_t flags);
extern uint32_t spin_trylock(spinlock_t* lock);
extern uint32_t spin_is_locked(spinlock_t* lock);
extern void spin_lock_unlock_wait(spinlock_t* lock);

#endif // _SPINLOCK_H_