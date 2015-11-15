#include <inc/x86/real.h>
#include <inc/klibs/palloc.h>
using namespace palloc;

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
    
    // Pass in the parameters.
    *(int16_t*)(REAL_MODE_DATA_BASE << 4) = interrupt_num;
    *(real_context_t*)((REAL_MODE_DATA_BASE << 4) + sizeof(int16_t)) = regs;
    legacyInt_noLock(interrupt_num);
    memcpy(&regs, &cpu0_real_context, sizeof(real_context_t));
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

RealModePtr::RealModePtr(const uint16_t ptr_arr[])
{
    off = ptr_arr[0];
    seg = ptr_arr[1];
}

RealModePtr::RealModePtr(uint16_t segment, uint16_t offset)
{
    off = offset;
    seg = segment;
}

void* RealModePtr::get32() const
{
    Maybe<uint32_t> base = virtOfPage0();
    if(base)
        return (void*)(!base + ((uint32_t) seg << 4) + (uint32_t) off);
    else return NULL;
}

// This function tries to find or create a virtual address for page 0MB - 4MB
// If it returns 0xffffffff, then memory is used up.
Maybe<uint32_t> RealModePtr::virtOfPage0() const
{
    // Search for a page from 0MB - 4MB (index=0)
    PhysAddr page0(0, PG_WRITABLE);

    VirtAddr base = cpu0_memmap.translate(page0);

    if(base.addr == NULL)
    {
        base = virtLast1G.allocPage(1);
        if(base.addr == NULL)
            return Maybe<uint32_t>();
        cpu0_memmap.addCommonPage(base, page0);
        return Maybe<uint32_t>((uint32_t)(base.addr));
    }
    return Maybe<uint32_t>((uint32_t)(base.addr));
}

