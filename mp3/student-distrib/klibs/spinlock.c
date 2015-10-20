#include <stdint.h>
#include <inc/spinlock.h>

void spin_lock_init(spinlock_t* lock)
{


}

void spin_lock_irqsave(spinlock_t* lock, uint32_t flags)
{


}

void spin_unlock_irqrestore(spinlock_t* lock, uint32_t flags)
{


}

uint32_t spin_trylock(spinlock_t* lock)
{
    return 0;
}

uint32_t spin_is_locked(spinlock_t* lock)
{
    return 1;
}

void spin_lock_unlock_wait(spinlock_t* lock)
{


}
