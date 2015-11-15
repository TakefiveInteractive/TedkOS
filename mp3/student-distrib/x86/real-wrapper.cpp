#include <inc/x86/real.h>

// Don't call this.
extern "C" void legacyInt_16bit_entry();

spinlock_t legacyInt_lock = SPINLOCK_UNLOCKED;

// Allocate 1KB for real_mode stack
volatile uint16_t real_mode_stack[512] __attribute__((aligned (1024)));

real_context_t cpu0_real_context;

void legacyInt(int16_t interrupt_num, real_context_t& regs)
{
    uint32_t flags;
    spin_lock_irqsave(&legacyInt_lock, flags);
    legacyInt_noLock(interrupt_num, &regs);
    spin_unlock_irqrestore(&legacyInt_lock, flags);
}

void prepareRealMode()
{
    uint32_t flags;
    spin_lock_irqsave(&legacyInt_lock, flags);

    // Firstly we ensure that ljmp $CS_16BIT_SEL, $0 will go to legacyInt_unreal_entry

    /*
    uint32_t base_addr = (uint32_t)legacyInt_16bit_entry;
    gdt[CS_16BIT_IDX].base_31_24 = (base_addr & 0xff000000) >> 24;
    gdt[CS_16BIT_IDX].base_23_16 = (base_addr & 0x00ff0000) >> 16;
    gdt[CS_16BIT_IDX].base_15_0  = (base_addr & 0x0000ffff);

    base_addr = (uint32_t)real_mode_stack;
    gdt[DS_16BIT_IDX].base_31_24 = (base_addr & 0xff000000) >> 24;
    gdt[DS_16BIT_IDX].base_23_16 = (base_addr & 0x00ff0000) >> 16;
    gdt[DS_16BIT_IDX].base_15_0  = (base_addr & 0x0000ffff);
    */

    // Then we copy legacyInt_16bit_entry to somewhere < 1MB
    memcpy((void*)(REAL_MODE_CODE_BASE << 4), (void*)legacyInt_16bit_entry, REAL_MODE_SEG_LEN);

    spin_unlock_irqrestore(&legacyInt_lock, flags);
}

extern "C" void __attribute__((used)) save_real_context(int16_t cpu, int16_t di, int16_t si, int16_t ax, int16_t bx, int16_t cx, int16_t dx)
{
    cpu0_real_context.ax = ax;
    cpu0_real_context.bx = bx;
    cpu0_real_context.cx = cx;
    cpu0_real_context.dx = dx;
    cpu0_real_context.di = di;
    cpu0_real_context.si = si;
    return;
}

extern "C" void __attribute__((used)) back_to_32bit()
{
    printf("Hello from kernel!\n");
    printf("Hello from kernel!\n");
    asm volatile("1: hlt; jmp 1b;");
}
