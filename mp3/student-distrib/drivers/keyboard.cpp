#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/keyboard.h>
#include <inc/d2d/to_term.h>
#include <inc/d2d/k2m.h>
#include <inc/klibs/spinlock.h>

#define KB_PORT 0x60
#define KB_INT_NUM 0x21
#define KB_IRQ_NUM 1
#define KB_ID 1
#define KD_POLICY 0

/*
 * See http://wiki.osdev.org/PS/2_Keyboard
 */
uint32_t KBascii[128] =
{
    /* keycode here is all pressed keycode */
    0,/* unused,nothing happened */
    KKC_ESC,/* esc */
	'1','2','3','4','5','6','7','8','9','0','-','=',
	KKC_BACKSPACE,/* Backspace */
	KKC_TAB,/* Tab */
	'q','w','e','r','t','y','u','i','o','p','[',']',
	KKC_ENTER,	/* Enter key */
    KKC_CTRL,	/* left Control */
	'a','s','d','f','g','h','j','k','l',
    ';',	/* 39 */
	'\'', '`',
	KKC_SHIFT,	/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',
	KKC_SHIFT,	/* Right shift */
	'*',
    KKC_ALT,	/* left Alt */
	' ',/* Space */
    KKC_CAPSLOCK,	/* Caps lock */
    KKC_F1, KKC_F2, KKC_F3, KKC_F4, KKC_F5, KKC_F6, KKC_F7, KKC_F8, KKC_F9, KKC_F10,
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    KKC_HOME,	/* Home key */
    KKC_UP,	/* Up Arrow */
    KKC_PAGEUP,	/* Page Up */
	'-',
    KKC_LEFT,	/* Left Arrow */
    0,
    KKC_RIGHT,	/* Right Arrow */
	'+',
    KKC_END,	/* 79 - End key*/
    KKC_DOWN,	/* Down Arrow */
    KKC_PAGEDOWN,	/* Page Down */
    KKC_INSERT,	/* Insert Key */
    KKC_DELETE,	/* Delete Key */
    0, 0, 0,
    KKC_F11,	/* F11 Key */
    KKC_F12,	/* F12 Key */
    0,	/* All other keys are undefined */
};

//--------------- Special key Prefix is 0xE0 ---------------
#define SPECIAL_PREFIX      0xE0
#define RELEASE_OFFSET      0x80

int8_t  pending_special = 0;
spinlock_t keyboard_lock = SPINLOCK_UNLOCKED;
extern "C" int kb_handler(int irq, unsigned int saved_reg);

DEFINE_DRIVER_INIT(kb) {
    uint32_t flag;
    spin_lock_irqsave(&keyboard_lock, flag);

	// bind handler to pic
	bind_irq(KB_IRQ_NUM,KB_ID,kb_handler,KD_POLICY);

    spin_unlock_irqrestore(&keyboard_lock, flag);
	return;
}

DEFINE_DRIVER_REMOVE(kb) {
    uint32_t flag;
    spin_lock_irqsave(&keyboard_lock, flag);

	//rm handler from pic
	unbind_irq(KB_IRQ_NUM,KB_ID);
    pending_special = 0;

    spin_unlock_irqrestore(&keyboard_lock, flag);
    return;
}

/* keyboard_handler
 * description:
 *		Handle keyboard interrupts
 * input:
 *		int irq, pt_reg* saved_reg(not used)
 * output,return:
 *		currently the value is ignored, 0 for success.
 *      Look at <inc/error.h> for more error codes.
 * side effect:
 *		changes keyboard buffer (if we have a buffer)
 *      print keyboard character to terminal
 */
int kb_handler(int irq, unsigned int saved_reg)
{
	uint8_t keyboard_scancode;
    uint32_t flag;
    spin_lock_irqsave(&keyboard_lock, flag);

    if ( (mouse_enable_scancode=inb(MOUSE_ENABLE_PORT) ) == 0x20) //should be handle by ms
    {
        spin_unlock_irqrestore(&keyboard_lock, flag);
        return 0;
    }
 	keyboard_scancode = inb(KB_PORT);                           //read the input

    //!!! WARNING: We should not use "keyboard_scancode & 0x80" anymore
    //!!!   Because that filters out the events for release keys
    //!!!   BUT we need to know when keys like SHIFT, ALT are released

    if(keyboard_scancode == 0) {
        spin_unlock_irqrestore(&keyboard_lock, flag);
        return 0;
    }
    if(keyboard_scancode == SPECIAL_PREFIX) {
        pending_special = 1;
        spin_unlock_irqrestore(&keyboard_lock, flag);
        return 0;
    }

    // On some keyboards, keypad enter's 2nd code is not 0x1C
    //   They use 0x18 as 2nd code, which conflicts with 'o'
    if(pending_special && keyboard_scancode == 0x18) {
        pending_special = 0;
        spin_unlock_irqrestore(&keyboard_lock, flag);
        return 0;
    }
    pending_special = 0;
    if ((keyboard_scancode & RELEASE_OFFSET) == 0 ) {                     //pressed
 		uint32_t kernel_keycode = KBascii[keyboard_scancode];
 		kb_to_term(kernel_keycode|KKC_PRESS);
 	}

 	if (keyboard_scancode & RELEASE_OFFSET) {                             //released
 		uint32_t kernel_keycode = KBascii[keyboard_scancode & (~RELEASE_OFFSET)];
 		kb_to_term(kernel_keycode|KKC_RELEASE);
 	}

    spin_unlock_irqrestore(&keyboard_lock, flag);

    return 0;
}

//------------------------- This part of KeyB driver converts Hardware Keycode to KKC -----------------------

// The entry point into C++ world for all keyboard events.
/* The param is PARTIAL kernelKeycode, because keyboard does not combine keys.
 *  'a' is ok. KKC_ENTER is ok,
 *  but no one will pass in SHIFT|'a'.
 */
void BasicShortcutFilter::handle(uint32_t kernelKeycode)
//void kb_to_term(uint32_t kernelKeycode)
{
    uint32_t flag;
    uint32_t ascii_part, special_part, combine_part;

    // *_part are booleans to represent the type of key being pressed, or released
    // The rules to extract the type are explained in inc/driver/kkc.h
    ascii_part   = kernelKeycode & 0x000000FF;
    special_part = kernelKeycode & 0x0000FF00;
    combine_part = kernelKeycode & 0x7FFF0000;

    // If this event represents releasing a key
    if((kernelKeycode & KKC_RELEASE) != 0)
    {
        // calculate change so that (pending_kc & change) is the NEW pending_kc
        // If neither part exists, nothing should change, thus default to ~0x0
        uint32_t change = 0xFFFFFFFF;

        /* WE ASSUME keyboard supports auto-repeat interrupts. */

        change &= ~combine_part;

        pending_kc &= change;

        for(size_t i = 0; i < numClients; i++)
            clients[i].keyUp(kernelKeycode);
    }
    else
    {
        // A rule: everytime a new ascii/special key is pressed,
        //     old pressed ascii/special key is discarded
        if(combine_part)
            pending_kc = pending_kc | combine_part;
        else if(special_part)
        {
            kernelKeycode = 0;
            if(!pending_kc)
                sp_kkc_handler_table[special_part >> 8](special_part);
            else if(special_part == KKC_CAPSLOCK)
                sp_kkc_handler_table[KKC_CAPSLOCK >> 8](special_part);
            // Currently we do NOT handle other cases of COMBINE+SPECIAL

            if(!handleShortcut(special_part))
                clients[currClient].keyDown(special_part);
        }
        else if(ascii_part)         // Avoid 0x0
        {
            if(!handleShortcut(pending_kc | ascii_part))
                clients[currClient].keyDown(pending_kc | ascii_part);
            // Currently this is the ONLY COMBINATION allowed.
            if(pending_kc == KKC_CTRL && ascii_part == 'l')
                clear_screen_nolock();
            else if((pending_kc & (~KKC_SHIFT)) == 0 && term_buf_pos < TERM_BUFFER_SIZE - 1) //-1 because '\n' takes space
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
                term_buf[term_buf_pos++] = rec;
            }
        }
    }
}
