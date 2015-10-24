#include <stdint.h>
#include <inc/spinlock.h>

void spin_lock_init(spinlock_t* lock)
{


}

void spin_lock(spinlock_t* lock)
{
     int check = 0;
     do {
     	asm volatile("movl %1, %0;"//the curr lock status
                    "movl %2, %%eax;"//move macro into eax
                    "xchgl %1, %%eax;"
 
     			: "=r"(check)/* output  */
     			: "r"(lock), "i"(SPINLOCK_LOCKED) /* input %1 , %2(SPINLOCK_LOCKED)*/
     			: "cc","%eax"/* clobbered register */
     			);
     } while(check);
}

void spin_unlock(spinlock_t* lock)
{
    do {
        asm volatile("movl %1,%%eax;"//move macro into eax
                    "xchgl %0, %%eax;"

     			: //"=r"(check)/* output  */
     			: "r"(lock), "i"(SPINLOCK_UNLOCKED) /* input %0 , %1(macro)*/
     			: "cc","%eax"
     			);
    } while(0);
}

void spin_lock_irqsave(spinlock_t* lock, uint32_t flag)
{

}

void spin_unlock_irqrestore(spinlock_t* lock, uint32_t flag)
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
