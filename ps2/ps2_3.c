/* Synchronization */

#include <linux/spinlock.h>

typedef struct food_drink_lock
{
	unsigned long drink_num;
	unsigned long food_num;
	spin_lock_t* lock;
} fd_lock_t;

void produce_food(fd_lock_t* fd) {
    unsigned long flags;
	spin_lock_irqsave(fd->lock, flags);
    fd->food_num++;
	spin_unlock_irqrestore(fd->lock, flags);
}

void consume_food(fd_lock_t* fd) {
	// When blocking this function, be careful to release all locks
	// , so as not to cause DEADLOCKS.

	// We must check all conditions in a single loop.
	// Because after we decide any condition is met,
	// we must keep the lock locked. And if now
	// any other condition fails, we have to either
	// cause a deadlock, or give up the lock (causing a race condition).
	
	unsigned long flags;
	
	while(true)
	{
		spin_lock_irqsave(fd->lock, flags);
		if(fd->drink_num == 0 && fd->food_num > 0)
		{
			// Here all conditions are met
			// And we must finish all required action without
			// blocking or releasing the lock.
			fd->food_num--;
			spin_unlock_irqrestore(fd->lock, flags);
			break;
		}
		spin_unlock_irqrestore(fd->lock, flags);
	}
	
	// There are only two situations in this function
	// Blocking -- which corresponds to code within 'while', outside 'if'
	// Finishing request -- which corresponds to code within 'if'
	// Therefore we don't write anything after 'while' here.
}

int produce_drink(fd_lock_t* fd) {
	int returnVal = 0;
	unsigned long flags;
	spin_lock_irqsave(fd->lock, flags);
	if(fd->drink_num < 10)
	{
		fd->drink_num++;
		returnVal = 0;
	}
	else returnVal = -1;
	spin_unlock_irqrestore(fd->lock, flags);
	return returnVal;
}

void consume_drink(d_lock_t* fd) {
	// In this function I will not explain about the loops again.
	
	// When conditions fail, we still have to block current thread.
	
	unsigned long flags;
	
	while(true)
	{
		spin_lock_irqsave(fd->lock, flags);
		if(fd->drink_num > 0)
		{
			fd->drink_num--;
			spin_unlock_irqrestore(fd->lock, flags);
			break;
		}
		spin_unlock_irqrestore(fd->lock, flags);
	}
}
