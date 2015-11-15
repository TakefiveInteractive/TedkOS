#include <inc/x86/paging.h>
#include <inc/ui/vbe.h>

// From inc/x86/real.h: If you want to use %es, use REAL_MODE_FREE_SEG

Maybe<VbeInfoBlock> getVbeInfo()
{
    real_context_t context;

    // Set output at physical address = REAL_MODE_FREE_SEG << 4 + 0
    context.es = REAL_MODE_FREE_SEG;
    context.di = 0;
    
    context.ax = 0x4f00;

    legacyInt(0x10, context);
    
    if(context.ax != 0x004f)
        return Maybe<VbeInfoBlock>();
    
    // We use VMEM to Access physical Page at 0x0
    return Maybe<VbeInfoBlock>(*(VbeInfoBlock*)((((uint32_t)video_mem) & ALIGN_4MB_ADDR) + (REAL_MODE_FREE_SEG << 4)));
}

