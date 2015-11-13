#include <inc/x86/real.h>

spinlock_t legacyInt_lock = SPINLOCK_UNLOCKED;

void legacyInt(int16_t interrupt_num, pusha_t& regs)
{
    uint32_t flags;
    spin_lock_irqsave(&legacyInt_lock, flags);
    legacyInt_noLock(interrupt_num, &regs);
    spin_unlock_irqrestore(&legacyInt_lock, flags);
}
