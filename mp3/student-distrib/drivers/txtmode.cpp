#include <inc/drivers/kbterm.h>
#include <inc/klibs/palloc.h>
#include <inc/init.h>

using palloc::virtOfPage0;

namespace Term
{
    TextModePainter* TextModePainter::currShowing = NULL;

    uint8_t* TextModePainter::videoMem()
    {
        auto page0Maybe = virtOfPage0();
        if(!page0Maybe)
            return NULL;

        uint32_t page0 = +page0Maybe;
        if(isLoadedInVmem)
            return (uint8_t*)(page0 + TXT_VMEM_OFF);
        else return backupBuffer;
    }

    void TextModePainter::clearScreenNolock()
    {
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "movl %1, %%eax                                         ;"
            "rep stosw    # reset ECX *word* from M[ESI] to M[EDI]  "
            : /* no outputs */
            : "i" (SCREEN_WIDTH * SCREEN_HEIGHT),
              "i" (' ' + (TEXT_STYLE << 8)),
              "D" (videoMem())
            : "cc", "memory", "ecx", "eax"
        );

        // ======== reset cursor position ========
        cursorX = 0;
        cursorY = 0;

        // If this terminal is currently hidden: do NOTHING but save cursor location
        if(!isLoadedInVmem)
            return;

        helpSetCursor(0, 0);
    }

    void TextModePainter::clearScreen()
    {
        AutoSpinLock l(&lock);
        clearScreenNolock();
    }

    // Scroll the whole screen down by 1 line.
    void TextModePainter::scrollDown()
    {
        AutoSpinLock l(&lock);

        // The width must be an even number so that
        //      scroll_down_nolock can use rep movsd

        // Move lines up
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
            : /* no outputs */
            : "i" ((SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2 / 4),
              "S" (videoMem() + 2 * SCREEN_WIDTH),
              "D" (videoMem())
            : "cc", "memory", "ecx"
        );

        // Clear the content of the last line.
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "movl %1, %%eax                                         ;"
            "rep stosw    # reset ECX *word* from M[ESI] to M[EDI]  "
            : /* no outputs */
            : "i" (SCREEN_WIDTH),
              "i" (' ' + (TEXT_STYLE << 8)),
              "D" (videoMem() + (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2)
            : "cc", "memory", "ecx", "eax"
        );
    }

    #define CURSOR_LOC_HIGH_REG     0x0E
    #define CURSOR_LOC_LOW_REG      0x0F

    void TextModePainter::helpSetCursor(uint32_t x, uint32_t y)
    {
        // old_addr stores old CRTC Register's address
        // addr_reg and data_reg are used to operate CRTC registers
        // location will be passed to VGA register as Cursor Location Field.
        uint8_t old_addr;
        uint16_t addr_reg, data_reg;
        uint16_t location = SCREEN_WIDTH * y + x;
        if(x >= SCREEN_WIDTH)
            return;
        if(y >= SCREEN_HEIGHT)
            return;
        if(inb(0x3CC) & 0x1)
        {
            addr_reg = 0x3D4;
            data_reg = 0x3D5;
        }
        else
        {
            addr_reg = 0x3B4;
            data_reg = 0x3B5;
        }
        old_addr = inb(addr_reg);
        outb(CURSOR_LOC_HIGH_REG, addr_reg);
        outb(location >> 8, data_reg);
        outb(CURSOR_LOC_LOW_REG , addr_reg);
        outb(location & 0xff, data_reg);
        outb(old_addr, addr_reg);
    }

    void TextModePainter::setCursor(uint32_t x, uint32_t y)
    {
        AutoSpinLock l(&lock);

        cursorX = x;
        cursorY = y;

        // If this terminal is currently hidden: do NOTHING but save cursor location
        if(!isLoadedInVmem)
            return;

        helpSetCursor(x, y);
    }

    void TextModePainter::showChar(uint32_t x, uint32_t y, uint8_t c)
    {
        AutoSpinLock l(&lock);
        *(uint8_t *)(videoMem() + ((SCREEN_WIDTH*y + x) << 1)) = c;
        *(uint8_t *)(videoMem() + ((SCREEN_WIDTH*y + x) << 1) + 1) = TEXT_STYLE;
    }

    void TextModePainter::clearLine(uint32_t y)
    {
        AutoSpinLock l(&lock);
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "movl %1, %%eax                                         ;"
            "rep stosw    # reset ECX *word* from M[ESI] to M[EDI]  "
            : // no outputs
            : "i" (SCREEN_WIDTH),
              "i" (' ' + (TEXT_STYLE << 8)),
              "D" (videoMem() + y * SCREEN_WIDTH * 2)
            : "cc", "memory", "ecx", "eax"
        );
    }

    void TextModePainter::show()
    {
        AutoSpinLock l(&lock);
        auto page0Maybe = virtOfPage0();
        if(!page0Maybe)
            return;
        uint32_t page0 = +page0Maybe;

        //TODO: FIXME: Should First use vbe.cpp to switch back to Text Mode .

        if(currShowing == this)
            return;

        if(currShowing)
        {
            AutoSpinLock theirLock(&currShowing->lock);
            asm volatile (
                "cld                                                    ;"
                "movl %0, %%ecx                                         ;"
                "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
                : /* no outputs */
                : "i" (SCREEN_HEIGHT * SCREEN_WIDTH * 2 / 4),
                  "S" ((uint8_t*)(page0 + TXT_VMEM_OFF)),
                  "D" (currShowing->backupBuffer)
                : "cc", "memory", "ecx"
            );
            currShowing->isLoadedInVmem = false;

            AutoSpinLock l2(&cpu0_paging_lock);
            currShowing->tryMapVidmapNolock();
        }
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
            : /* no outputs */
            : "i" (SCREEN_HEIGHT * SCREEN_WIDTH * 2 / 4),
              "S" (backupBuffer),
              "D" ((uint8_t*)(page0 + TXT_VMEM_OFF))
            : "cc", "memory", "ecx"
        );
        currShowing = this;
        isLoadedInVmem = true;
        helpSetCursor(cursorX, cursorY);
    }

    bool TextModePainter::canShowVidmap()
    {
        return isLoadedInVmem && bIsVidmapEnabled;
    }

    void TextModePainter::tryMapVidmapNolock()
    {
        if(!pcbLoadable || !canUseCpp || isFallbackTerm)
            return;
        if(bIsVidmapEnabled)
        {
            uint32_t addr;
            if(isLoadedInVmem)
                addr = PRE_INIT_VIDEO;
            else addr = (uint32_t) backupBuffer;
            userFirst4MBTable[PRE_INIT_VIDEO >> 12] = (uint32_t) PG_4KB_BASE | PG_WRITABLE | PG_USER | (addr & ALIGN_4KB_ADDR);
            LOAD_PAGE_TABLE(0, userFirst4MBTable, PT_WRITABLE | PT_USER);
            RELOAD_CR3();
        }
        else
        {
            global_cr3val[0] = 0;
            RELOAD_CR3();
        }
    }

    void TextModePainter::tryMapVidmap()
    {
        AutoSpinLock l(&lock);
        tryMapVidmapNolock();
    }

    TextModePainter::TextModePainter() : TermPainter()
    {
        clearScreen();
    }

    uint8_t* TextModePainter::enableVidmap()
    {
        AutoSpinLock l(&lock);
        clearScreenNolock();
        bIsVidmapEnabled = true;
        LOAD_PAGE_TABLE(0, userFirst4MBTable, PT_WRITABLE | PT_USER);
        RELOAD_CR3();
        return (uint8_t*)PRE_INIT_VIDEO;
    }

    void TextModePainter::tryDisableVidmap()
    {
        AutoSpinLock l(&lock);
        bIsVidmapEnabled = false;
        global_cr3val[0] = 0x0;
        RELOAD_CR3();
        return;
    }

    bool TextModePainter::isVidmapEnabled()
    {
        return bIsVidmapEnabled;
    }
}

