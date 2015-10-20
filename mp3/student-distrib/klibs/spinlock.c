#include <stdint.h>
#include <inc/spinlock.h>

void spin_lock_init(spinlock_t* lock){


}

void spin_lock_lock_irqsave(spinlock_t* lock, unsigned long flags){


}

void spin_lock_unlock_irqrestore(spinlock_t* lock, unsigned long flags){


}

int spin_trylock(spinlock_t* lock){


}

int spin_is_locked(spinlock_t* lock){


}

void spin_lock_unlock_wait(spinlock_t* lock){


}
