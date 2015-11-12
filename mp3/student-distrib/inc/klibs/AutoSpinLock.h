#ifndef _INC_AUTOSPINLOCK_H_
#define _INC_AUTOSPINLOCK_H_

#include <stdint.h>
#include <inc/klibs/spinlock.h>

class NonCopyable
{
    private:
        NonCopyable(NonCopyable const &); // private copy constructor
        NonCopyable& operator= (NonCopyable const &); // private assignment operator
    public:
        NonCopyable() { }
};

class AutoSpinLock: NonCopyable // Scoped Lock idiom
{
    public:
        AutoSpinLock(spinlock_t *l) : lock_(l)
        {
            uint32_t flag;
            spin_lock_irqsave(lock_, flag);
            flag_ = flag;
        }
        ~AutoSpinLock() throw()
        {
            uint32_t flag = flag_;
            spin_unlock_irqrestore(lock_, flag);
        }
    private:
        spinlock_t *lock_;
        uint32_t flag_;
};

#endif
