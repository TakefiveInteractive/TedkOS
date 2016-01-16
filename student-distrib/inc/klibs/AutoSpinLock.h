#ifndef _INC_AUTOSPINLOCK_H_
#define _INC_AUTOSPINLOCK_H_

#include <stdint.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/function.h>

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

        void waitUntil(function<bool (void)> fn)
        {
            uint32_t flag = flag_;
            spin_unlock_irqrestore(lock_, flag);
            for (;;)
            {
                spin_lock_irqsave(lock_, flag);
                if(fn()) break;
                spin_unlock_irqrestore(lock_, flag);
            }
            flag_ = flag;
        }
    private:
        spinlock_t *lock_;
        uint32_t flag_;
};

class AutoSpinLockKeepIF: NonCopyable // Scoped Lock idiom, without changing Interrupt Flag
{
    public:
        AutoSpinLockKeepIF(spinlock_t *l) : lock_(l)
        {
            spin_lock(lock_);
        }
        ~AutoSpinLockKeepIF() throw()
        {
            spin_unlock(lock_);
        }

        void waitUntil(function<bool (void)> fn)
        {
            spin_unlock(lock_);
            for (;;)
            {
                spin_lock(lock_);
                if(fn()) break;
                spin_unlock(lock_);
            }
        }
    private:
        spinlock_t *lock_;
};

#endif
