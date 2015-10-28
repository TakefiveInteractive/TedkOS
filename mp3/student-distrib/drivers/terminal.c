#include <stdint.h>
#include <stddef.h>
#include <inc/driver.h>
#include <inc/lib.h>
#include <inc/spinlock.h>

// The width must be an even number so that
//      scroll_down_nolock can use rep movsd
#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25
#define TEXT_STYLE      0x7
#define VMEM_HEAD       ((char*)0xB8000)

// The coordinate to display the next char at.
static uint32_t next_char_x = 0;
static uint32_t next_char_y = 0;
static char* video_mem = VMEM_HEAD;

// The lock must be called when operating on the screen.
static spinlock_t screen_lock = SPINLOCK_UNLOCKED;

#define TERM_BUFFER_SIZE                128

// We assume that read() should also be able to
//  return key combinations and special keys
//      Thus we have to store whole kernel keycode
//  But I am still waiting for Piazza's response.
uint32_t* term_buffer[TERM_BUFFER_SIZE];

// the position where new typed
// characters should go in buffer.
int buffer_position;

/********** Private, yet debuggable functions ***********/
void scroll_down_nolock();
void set_cursor_nolock(uint32_t x, uint32_t y);

/********** Private functions ***********/

/*
 *   Inputs:
 *       uint_8* c = PRINTABLE ascii character
 *       uint_32 x : the x coordinate to display at
 *       uint_32 y : the y coordinate to display at
 *   Return Value: void
 *	Function: Output a character to the screen
 *  WARNING: lock screen_lock WHEN CALLING !!!
 */
static inline void show_char_at_nolock(uint32_t x, uint32_t y, uint8_t c)
{
    *(uint8_t *)(video_mem + ((SCREEN_WIDTH*y + x) << 1)) = c;
    *(uint8_t *)(video_mem + ((SCREEN_WIDTH*y + x) << 1) + 1) = TEXT_STYLE;
}

/********** Public functions ***********/

DEFINE_DRIVER_INIT(term)
{
    // Initialize the buffer.
    buffer_position = 0;
}

DEFINE_DRIVER_REMOVE(term)
{
    // Currently nothing to do here.
}

void kb_to_term(uint32_t kernelKeycode)
{
    // This one is NOT a FINAL design.
    uint32_t flag;
    spin_lock_irqsave(&screen_lock, flag);
    if(kernelKeycode == KKC_ENTER) {
        if(next_char_y < SCREEN_HEIGHT - 1)
        {
            next_char_y++;
            next_char_x = 0;
        }
        else
        {
            next_char_x = 0;
            scroll_down_nolock();
        }
    } else if (!(kernelKeycode & 0xFFFFFF00)) {
        // In this case we have directly printable characters
        char c = (char)kernelKeycode;
        show_char_at_nolock(next_char_x, next_char_y, c);
        next_char_x++;
        if(next_char_x == SCREEN_WIDTH)
        {
            next_char_x = 0;
            if(next_char_y < SCREEN_HEIGHT - 1)
                next_char_y++;
            else scroll_down_nolock();
        }
    }
    set_cursor_nolock(next_char_x, next_char_y);
    spin_unlock_irqrestore(&screen_lock, flag);
}

// clear screen
void term_cls(void)
{
    uint32_t flag;
    spin_lock_irqsave(&screen_lock, flag);
    asm volatile (
        "cld                                                    ;"
        "movl %0, %%ecx                                         ;"
        "movl %1, %%eax                                         ;"
        "rep stosw    # reset ECX *word* from M[ESI] to M[EDI]  "
        : /* no outputs */
        : "i" (SCREEN_WIDTH * SCREEN_HEIGHT),
          "i" (' ' + (TEXT_STYLE << 8)),
          "D" (video_mem)
        : "cc", "memory", "ecx", "eax"
    );
    next_char_x = next_char_y = 0;
    spin_unlock_irqrestore(&screen_lock, flag);
}

// Print one char. Must be either printable or newline character
void term_putc(uint8_t c)
{
    uint32_t flag;
    spin_lock_irqsave(&screen_lock, flag);
    if(c == '\n' || c == '\r') {
        if(next_char_y < SCREEN_HEIGHT - 1)
        {
            next_char_y++;
            next_char_x = 0;
        }
        else
        {
            next_char_x = 0;
            scroll_down_nolock();
        }
    } else {
        show_char_at_nolock(next_char_x, next_char_y, c);
        next_char_x++;
        if(next_char_x == SCREEN_WIDTH)
        {
            next_char_x = 0;
            if(next_char_y < SCREEN_HEIGHT - 1)
                next_char_y++;
            else scroll_down_nolock();
        }
    }
    set_cursor_nolock(next_char_x, next_char_y);
    spin_unlock_irqrestore(&screen_lock, flag);
}

/********** Implementation of Private Functions ***********/

#define CURSOR_LOC_HIGH_REG     0x0E
#define CURSOR_LOC_LOW_REG      0x0F

/*
 * void set_cursor_nolock(uint32_t x, uint32_t y)
 * Inputs:
 *      x and y must be within correct range,
 *      (see SCREEN_WIDTH and SCREEN_HEIGHT)
 *      if they are out of range, nothing will happen
 * Function: moves cursor to screen location (x,y).
 * WARNING: lock the screen_lock !!!
 */
void set_cursor_nolock(uint32_t x, uint32_t y)
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

// Scroll the whole screen down by 1 line.
// There is a VGA way to do this. But we don't have time now.
// WARNING: lock the screen_lock !!!
void scroll_down_nolock()
{
    // The width must be an even number so that
    //      scroll_down_nolock can use rep movsd

    // Move lines up
    asm volatile (
        "cld                                                    ;"
        "movl %0, %%ecx                                         ;"
        "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
        : /* no outputs */
        : "i" ((SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2 / 4),
          "S" (video_mem + 2 * SCREEN_WIDTH),
          "D" (video_mem)
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
          "D" (video_mem + (SCREEN_HEIGHT - 1) * SCREEN_WIDTH * 2)
        : "cc", "memory", "ecx", "eax"
    );
}

