#include <stdint.h>
#include <stddef.h>
#include <inc/terminal.h>
#include <inc/driver.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/spinlock.h>
#include <inc/d2d/to_kb.h>
#include <inc/d2d/to_term.h>

#include <inc/fops_kb.h>
#include <inc/fops_term.h>

// The width must be an even number so that
//      scroll_down_nolock can use rep movsd.
#define TAB_WIDTH       4
#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25
#define TEXT_STYLE      0x7
#define VMEM_HEAD       ((char*)0xB8000)

// This field is used to tell whether COMBINATION key is pressed
//      If so, then it also contains all COMBINATION keys pressed.
//      Note that pending_kc's MSB is never 1 (released)
static uint32_t pending_kc = 0;

static uint8_t caps_locked = 0;

// The coordinate to display the next char at.
static uint32_t next_char_x = 0;
static uint32_t next_char_y = 0;

// This variable must be initialized every time _init() is called.
//  Later we might use VGA panning so this value might change.
static char* video_mem = VMEM_HEAD;

// The lock must be called when operating on the screen.
spinlock_t term_lock = SPINLOCK_UNLOCKED;

#define RINGBUF_SIZE            TERM_BUFFER_SIZE
#define RINGBUF_TYPE            term_buf_item
#include <inc/klibs/ringbuf.h>

// read(keyboard) should return displayed contents
// But it does not rely on term_DELETE_buf.
// !!! read(keyboard) is MANAGED BY TERMINAL's ANOTHER buffer !!!!
//      This buffer is used to indicate:
//        how much offset is when deleting a character
ringbuf_t term_delete_buf;

// the buffer used by fops
ringbuf_t term_read_buf;

/********** Private, yet debuggable functions ***********/
void scroll_down_nolock(void);
void set_cursor_nolock(uint32_t x, uint32_t y);
void clear_screen_nolock(void);

/********** Private functions ***********/

/*
 *   Inputs:
 *       uint_8* c = PRINTABLE ascii character
 *       uint_32 x : the x coordinate to display at
 *       uint_32 y : the y coordinate to display at
 *   Return Value: void
 *	Function: Output a character to the screen
 *  WARNING: lock term_lock WHEN CALLING !!!
 */
static inline void show_char_at_nolock(uint32_t x, uint32_t y, uint8_t c)
{
    *(uint8_t *)(video_mem + ((SCREEN_WIDTH*y + x) << 1)) = c;
    *(uint8_t *)(video_mem + ((SCREEN_WIDTH*y + x) << 1) + 1) = TEXT_STYLE;
}

/********** FOpsTable values ***********/
#include <inc/fs/fops.h>
#include <inc/fs/dev_wrapper.h>

FOpsTable fops_term = {
    .open = term_open,
    .close = term_close,
    .write = term_write,
    .read = term_read
};

FOpsTable fops_kb = {
    .open = kb_open,
    .close = kb_close,
    .write = kb_write,
    .read = kb_read
};

/********** Public functions ***********/

DEFINE_DRIVER_INIT(term)
{
    uint32_t flag;
    spin_lock_irqsave(&term_lock, flag);

    // Initialize the buffer.
    RINGBUF_INIT(&term_delete_buf);
    RINGBUF_INIT(&term_read_buf);

    register_devfs("/dev/term", fops_term);
    register_devfs("/dev/keyb", fops_kb);

    spin_unlock_irqrestore(&term_lock, flag);
}

DEFINE_DRIVER_REMOVE(term)
{
    uint32_t flag;
    spin_lock_irqsave(&term_lock, flag);

    // 0 means no key is pressed or release, nothing happened.
    pending_kc = 0;

    caps_locked = 0;

    clear_screen_nolock();
    next_char_x = 0;
    next_char_y = 0;
    video_mem = VMEM_HEAD;

    spin_unlock_irqrestore(&term_lock, flag);
}

//********** DEFINE HANDLERS and DISPATCHER for keycodes **********

// special kernel keycode (but not combine-able) handler
// such as ENTER, BACKSPACE, DELETE
// actually we will declere one handler per key
// But incase we need to differentiate KEY_DOWN and KEY_UP,
//      We still passes the kernelKeycode to handler
typedef void (*sp_kkc_handler)(uint32_t kernelKeycode);

// jump table composed of sp_kkc_handler, defined in terminal-asm.S
// Warning: these handlers DO NOT LOCK spinlocks !!!
extern sp_kkc_handler sp_kkc_handler_table[NUM_SPECIAL_KKC];
extern char ascii_shift_table[128];

/* IMPORTANT!! you can only pass one part at a time
 *  'a' is ok. KKC_ENTER is ok,
 *  but SHIFT|'a' is not ok.
 */
void kb_to_term(uint32_t kernelKeycode)
{
    // This one is NOT a FINAL design.
    uint32_t flag;
    uint32_t ascii_part, special_part, combine_part;
    spin_lock_irqsave(&term_lock, flag);

    ascii_part   = kernelKeycode & 0x000000FF;
    special_part = kernelKeycode & 0x0000FF00;
    combine_part = kernelKeycode & 0x7FFF0000;

    if((kernelKeycode & 0x80000000) == KKC_RELEASE)
    {
        // calculate change so that (pending_kc & change) is the NEW pending_kc
        // If neither part exists, nothing should change, thus default to ~0x0
        uint32_t change = 0xFFFFFFFF;

        /* WE ASSUME keyboard supports auto-repeat interrupts. */

        change &= ~combine_part;

        pending_kc &= change;
    }
    else
    {
        // Simulate linux: everytime a new key is pressed,
        //     old pressed ascii/special key is discarded
        if(combine_part)
            pending_kc = pending_kc | combine_part;
        else if(special_part)
        {
            if(!pending_kc)
                sp_kkc_handler_table[special_part >> 8](kernelKeycode);
            else if(special_part == KKC_CAPSLOCK)
                sp_kkc_handler_table[KKC_CAPSLOCK >> 8](kernelKeycode);
            // Currently we do NOT handle other cases of COMBINE+SPECIAL
        }
        else if(ascii_part)         // Avoid 0x0
        {
            // Currently this is the ONLY COMBINATION allowed.
            if(pending_kc == KKC_CTRL && ascii_part == 'l')
                clear_screen_nolock();
            else if((pending_kc & (~KKC_SHIFT)) == 0)
            {
                // In this case we have directly printable characters
                char c = (char)ascii_part;
                uint32_t old_x;
                term_buf_item rec;
                if(pending_kc & KKC_SHIFT)
                {
                    // If this ascii has a shift, then do shift.
                    if(ascii_shift_table[ascii_part])
                        c = ascii_shift_table[ascii_part];
                }
                if(caps_locked)
                {
                    if('a' <= c && c <= 'z')
                        c = c - 'a' + 'A';
                    else if('A' <=c && c <= 'Z')
                        c = c - 'A' + 'a';
                }
                show_char_at_nolock(next_char_x, next_char_y, c);
                rec.displayed_char = c;
                old_x = next_char_x;
                next_char_x++;
                if(next_char_x == SCREEN_WIDTH)
                {
                    rec.x_offset = SCREEN_WIDTH - old_x;
                    rec.y_offset = 1;
                    next_char_x = 0;
                    if(next_char_y < SCREEN_HEIGHT - 1)
                        next_char_y++;
                    else scroll_down_nolock();
                }
                else
                {
                    rec.x_offset = next_char_x - old_x;
                    rec.y_offset = 0;
                }
                ringbuf_push(&term_delete_buf, &rec);
                ringbuf_push(&term_read_buf, &rec);
            }
        }

        // Only pressing events can put OR remove any text onto screen.
        // Thus set_cursor_nolock is called within this 'else'
        set_cursor_nolock(next_char_x, next_char_y);
    }
    spin_unlock_irqrestore(&term_lock, flag);
}

//--------------- Definition of sp_kkc_handlers. These must match with terminal-asm.S !!! ----------------

// Warning: these handlers DO NOT LOCK spinlocks !!!
void term_enter_handler(uint32_t keycode)
{
    // newLine is used by term_read_buf, so offset_? is not needed
    term_buf_item newLine;
    newLine.displayed_char = '\n';

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
    RINGBUF_INIT(&term_delete_buf);
    ringbuf_push(&term_read_buf, &newLine);

    // currently we only have 1 terminal. CP2
    term2kb_readover(0);
}

void term_backspace_handler(uint32_t keycode)
{
    if(!ringbuf_is_empty(&term_read_buf))
    {
        term_buf_item* i = ringbuf_back_nocp(&term_read_buf);
        if(i->displayed_char != '\n')
            ringbuf_pop_back(&term_read_buf);
    }
    if(!ringbuf_is_empty(&term_delete_buf))
    {
        term_buf_item* i = ringbuf_back_nocp(&term_delete_buf);
        if(i->y_offset)
        {
            // Clear current line
            asm volatile (
                "cld                                                    ;"
                "movl %0, %%ecx                                         ;"
                "movl %1, %%eax                                         ;"
                "rep stosw    # reset ECX *word* from M[ESI] to M[EDI]  "
                : /* no outputs */
                : "i" (SCREEN_WIDTH),
                  "i" (' ' + (TEXT_STYLE << 8)),
                  "D" (video_mem + next_char_y * SCREEN_WIDTH * 2)
                : "cc", "memory", "ecx", "eax"
            );
            next_char_y--;
            next_char_x = SCREEN_WIDTH - i->x_offset;

            // Must only use this direction (j-- cause sign issues)
            for(uint32_t j = next_char_x; j <= SCREEN_WIDTH - 1; j++)
                show_char_at_nolock(j, next_char_y, ' ');
        }
        else
        {
            uint32_t orig_x = next_char_x;
            next_char_x = orig_x - i->x_offset;

            // Must only use this direction (j-- cause sign issues)
            for(uint32_t j = next_char_x; j <= orig_x; j++)
                show_char_at_nolock(j, next_char_y, ' ');
        }
        ringbuf_pop_back(&term_delete_buf);
    }
    set_cursor_nolock(next_char_x, next_char_y);
}

void term_capslock_handler(uint32_t keycode)
{
    caps_locked = !caps_locked;
}

void term_delete_handler(uint32_t keycode)
{
    ;
}

void term_tab_handler(uint32_t keycode)
{
    uint32_t old_x;
    term_buf_item rec;
    rec.displayed_char = '\t';
    old_x = next_char_x;
    next_char_x  += 4 - (next_char_x % TAB_WIDTH);
    if(next_char_x >= SCREEN_WIDTH)
    {
        rec.x_offset = SCREEN_WIDTH - old_x;
        rec.y_offset = 1;

        // Linux VM is bad at this.
        // I decided to make any tab start at newline.
        next_char_x = 0;
        if(next_char_y < SCREEN_HEIGHT - 1)
            next_char_y++;
        else scroll_down_nolock();
    }
    else
    {
        rec.x_offset = next_char_x - old_x;
        rec.y_offset = 0;
    }
    ringbuf_push(&term_delete_buf, &rec);
    ringbuf_push(&term_read_buf,   &rec);
}

void term_none_handler(uint32_t keycode)
{
    ;
}

//-------------------- VGA operations ----------------------

// this function DOES move cursor or next_char_*
void clear_screen_nolock(void)
{
    // Re-Initialize the DELETE buffer.
    RINGBUF_INIT(&term_delete_buf);

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
    set_cursor_nolock(next_char_x, next_char_y);
}

// clear screen
void term_cls(void)
{
    uint32_t flag;
    spin_lock_irqsave(&term_lock, flag);
    clear_screen_nolock();
    spin_unlock_irqrestore(&term_lock, flag);
}

// Print one char. Must be either printable or newline character
void term_putc(uint8_t c)
{
    uint32_t flag;
    spin_lock_irqsave(&term_lock, flag);

    // Re-Initialize the DELETE buffer. (Things printed from screens are not deletable)
    RINGBUF_INIT(&term_delete_buf);

    if(c == '\0') {
        // We should not print spaces for any trailing null characters
        ;
    } else if(c == '\n' || c == '\r') {
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
    } else if (c == '\t') {
        next_char_x += 4 - (next_char_x % TAB_WIDTH);
        if(next_char_x >= SCREEN_WIDTH)
        {
            next_char_x = next_char_x % SCREEN_WIDTH;
            if(next_char_y < SCREEN_HEIGHT - 1)
                next_char_y++;
            else scroll_down_nolock();
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
    spin_unlock_irqrestore(&term_lock, flag);
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
 * WARNING: lock the term_lock !!!
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
// WARNING: lock the term_lock !!!
void scroll_down_nolock(void)
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

