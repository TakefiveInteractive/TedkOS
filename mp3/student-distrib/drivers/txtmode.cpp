#include <inc/drivers/kbterm.h>

namespace Term
{
    uint8_t* TextModePainter::videoMem()
    {
        if(isLoadedInVmem)
            return (uint8_t*)(virtOfPage0() + TXT_VMEM_OFF);
        else return backupBuffer;
    }

    void TextModePainter::clearScreen()
    {
        AutoSpinLock l(&lock);
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
        setCursor(0, 0);
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

    bool TextModePainter::show()
    {
        AutoSpinLock l(&lock);
        if(currShowing)
        {
            AutoSpinLock theirLock(&currShowing->lock);
            asm volatile (
                "cld                                                    ;"
                "movl %0, %%ecx                                         ;"
                "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
                : /* no outputs */
                : "i" (SCREEN_HEIGHT * SCREEN_WIDTH * 2 / 4),
                  "S" ((uint8_t*)(virtOfPage0() + TXT_VMEM_OFF)),
                  "D" (currShowing->backupBuffer)
                : "cc", "memory", "ecx"
            );
            currShowing->isLoadedInVmem = false;
        }
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
            : /* no outputs */
            : "i" (SCREEN_HEIGHT * SCREEN_WIDTH * 2 / 4),
              "S" (backupBuffer),
              "D" ((uint8_t*)(virtOfPage0() + TXT_VMEM_OFF))
            : "cc", "memory", "ecx"
        );
        currShowing->isLoadedInVmem = true;
        helpSetCursor(cursorX, cursorY);
    }

    TextModePainter::TextModePainter() : TermPainter()
    {
        clearScreen();
    }
}

