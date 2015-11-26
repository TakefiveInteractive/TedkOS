#include <stdint.h>#include <stddef.h>
#include <inc/drivers/terminal.h>
#include <inc/drivers/common.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/spinlock.h>
#include <inc/d2d/to_kb.h>
#include <inc/d2d/to_term.h>

#include <inc/drivers/fops_kb.h>
#include <inc/drivers/fops_term.h>

// The width must be an even number so that
//      scroll_down_nolock can use rep movsd.

/*
// This field is used to tell whether COMBINATION key is pressed
//      If so, then it also contains all COMBINATION keys pressed.
//      Note that pending_kc's MSB is never 1 (released)
static uint32_t pending_kc = 0;

static uint8_t caps_locked = 0;
*/

// The coordinate to display the next char at.
static uint32_t next_char_x = 0;
static uint32_t next_char_y = 0;

// The lock must be called when operating on the screen.
spinlock_t term_lock = SPINLOCK_UNLOCKED;

term_buf_item term_buf[TERM_BUFFER_SIZE];

// this is actually the position of NEXT NEW char.
volatile int32_t term_buf_pos = 0;

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
    term_buf_pos = 0;

    register_devfs("term", fops_term);
    register_devfs("keyb", fops_kb);

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
    term_buf_pos = 0;
    next_char_x = 0;
    next_char_y = 0;

    spin_unlock_irqrestore(&term_lock, flag);
}

//********** DEFINE HANDLERS for special keycodes **********
extern char ascii_shift_table[128];

// Warning: these handlers DO NOT LOCK spinlocks !!!
void Term::nolock_enter_handler(uint32_t keycode)
{
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
        getTermPainter()->scrollDown();
    }
    //  kb_read must add '\n' by itself.
    term_buf[term_buf_pos++] = newLine;
    UserWaitingRead = false;
}

void Term::nolock_backspace_handler(uint32_t keycode)
{
    if(term_buf_pos > 0)
    {
        term_buf_item* i = &term_buf[term_buf_pos - 1];
        if(i->y_offset)
        {
            /*
            // Clear current line (clearLine)
            asm volatile (
                "cld                                                    ;"
                "movl %0, %%ecx                                         ;"
                "movl %1, %%eax                                         ;"
                "rep stosw    # reset ECX *word* from M[ESI] to M[EDI]  "
                : // no outputs
                : "i" (SCREEN_WIDTH),
                  "i" (' ' + (TEXT_STYLE << 8)),
                  "D" (video_mem + next_char_y * SCREEN_WIDTH * 2)
                : "cc", "memory", "ecx", "eax"
            );
            */
            getTermPainter()->clearLine(next_char_y);
            next_char_y--;
            next_char_x = SCREEN_WIDTH - i->x_offset;

            // Must only use this direction (j-- cause sign issues)
            for(uint32_t j = next_char_x; j <= SCREEN_WIDTH - 1; j++)
                getTermPainter()->showChar(j, next_char_y, ' ');
        }
        else
        {
            uint32_t orig_x = next_char_x;
            next_char_x = orig_x - i->x_offset;

            // Must only use this direction (j-- cause sign issues)
            for(uint32_t j = next_char_x; j <= orig_x; j++)
                getTermPainter()->showChar(j, next_char_y, ' ');
        }
        term_buf_pos --;
    }
}

void Term::nolock_tab_handler(uint32_t keycode)
{
    if(term_buf_pos >= TERM_BUFFER_SIZE - 1)
        return;
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
        else getTermPainter()->scrollDown();
    }
    else
    {
        rec.x_offset = next_char_x - old_x;
        rec.y_offset = 0;
    }
    term_buf[term_buf_pos++] = rec;
}

//-------------------- VGA operations ----------------------

// this function DOES move cursor or next_char_*
void clear_screen_nolock(void)
{
    // Re-Initialize the DELETE buffer.
    term_buf_pos = 0;

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

// Print one char. Must be either printable or newline character
void term_putc(uint8_t c)
{
    uint32_t flag;
    spin_lock_irqsave(&term_lock, flag);

    // Re-Initialize the DELETE buffer. (Things printed from screens are not deletable)
    term_buf_pos = 0;

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

// -------- C++ classes ------------
void Term::key(uint32_t kkc, bool capslock)
{
    if(kkc & ~KKC_ASCII_MASK & ~KKC_SHIFT)
    {
        // Handle possible shortcuts here.
        if(kkc & (~KKC_SPECIAL_MASK))
        {
            if(kkc == (KKC_CTRL | (uint32_t)'l'))
            {
                next_char_x = next_char_y = 0;
                term_buf_pos = 0;
                getTermPainter()->clearScreen();
            }
        }
        else switch(kkc & KKC_SPECIAL_MASK)
        {
        case KKC_BACKSPACE:
            nolock_backspace_handler(kkc);
            break;
        case KKC_TAB:
            nolock_tab_handler(kkc);
            break;
        case KKC_ENTER:
            nolock_enter_handler(kkc);
            break;
        default:
            break;
        }
    }
    else
    {
        // Handle normal characters here.
        // < SIZE -1 because '\n' takes space
        if(term_buf_pos < TERM_BUFFER_SIZE - 1)
        {
            // In this case we have directly printable characters
            char c = (char)(kkc & KKC_ASCII_MASK);
            uint32_t old_x;
            term_buf_item rec;
            if(kkc & KKC_SHIFT)
            {
                // If this ascii has a shift, then do shift.
                if(ascii_shift_table[ascii_part])
                    c = ascii_shift_table[ascii_part];
            }
            if(capslock)
            {
                if('a' <= c && c <= 'z')
                    c = c - 'a' + 'A';
                else if('A' <=c && c <= 'Z')
                    c = c - 'A' + 'a';
            }

            getTermPainter()->showChar(next_char_x, next_char_y, c);

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
                else getTermPainter()->scrollDown();
            }
            else
            {
                rec.x_offset = next_char_x - old_x;
                rec.y_offset = 0;
            }
            term_buf[term_buf_pos++] = rec;
        }
    }
    getTermPainter()->setCursor(next_char_x, next_char_y);
}

