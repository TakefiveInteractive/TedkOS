#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "types.h"

#define SPINLOCK_LOCKED 1
#define SPINLOCK_UNLOCKED 0

typedef int spinlock_t;

extern void spin_lock_init(spinlock_t* lock);
extern void spin_lock_lock_irqsave(spinlock_t* lock, unsigned long flags);
extern void spin_lock_unlock_irqrestore(spinlock_t* lock, unsigned long flags);
extern int spin_trylock(spinlock_t* lock);
extern int spin_is_locked(spinlock_t* lock);
extern void spin_lock_unlock_wait(spinlock_t* lock);

#endif // _SPINLOCK_H_