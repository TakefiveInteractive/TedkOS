#include <stdint.h>
#include <inc/spinlock.h>

void spin_lock_init(spinlock_t* lock)
{
    *lock = SPINLOCK_UNLOCKED;
}

void spin_lock(spinlock_t* lock)
{
    asm volatile(
        "movl %1, %%eax                     ;"
        "1: lock xchgl (%0), %%eax          ;"
        "cmpl %%eax, %1                     ;"
        "je 1b                              ;"
        :
        : "r"(lock), "i"(SPINLOCK_LOCKED)   /* input */
        : "cc", "eax", "memory");
}

void spin_unlock(spinlock_t* lock)
{
    *lock = SPINLOCK_UNLOCKED;
}

// 1 on success. 0 on failure.
uint32_t spin_trylock(spinlock_t* lock)
{
    register uint32_t temp = SPINLOCK_LOCKED;
    asm volatile(
        "lock xchgl (%1), %0                ;"
        "cmpl %0, %2                        ;"
        "je 1f                              ;"
        "movl $1, %0                        ;"
        "jmp 2f                             ;"
        "1: movl $0, %0                     ;"
        "2:                                 ;"
        : "+r"(temp)
        : "r"(lock), "i"(SPINLOCK_LOCKED)
        : "cc", "eax", "memory");
    return temp;
}

uint32_t spin_is_locked(spinlock_t* lock)
{
    return *lock == SPINLOCK_LOCKED;
}
