#include <stdint.h>
#include <stddef.h>
#include <inc/keyboard.h>
#include <inc/d2d/to_term.h>

#define KB_PORT 0x60
#define KB_INT_NUM 0x21
#define KB_IRQ_NUM 1
#define KB_ID 1
#define KD_POLICY 0

uint32_t KBascii[128] =
{
    0,  27,/* esc */
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'-', '=',
	KKC_BACKSPACE,/* Backspace */
	'\t',/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	KKC_ENTER,	/* Enter key */
    0,	/* 29   - Control */        // PLEASE COMPLETE THE REST of the TABLE 
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',
	0,	/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',
	0,	/* Right shift */
	'*',
    0,	/* Alt */
	' ',/* Space */
    0,	/* Caps lock */
    0,
	/* 59 - F1 key ... > */
    0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
	'-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
	'+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    '\177',	/* Delete Key */
    0, 0, 0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

int kb_handler(int irq, unsigned int saved_reg);

DEFINE_DRIVER_INIT(kb) {
	// bind handler to pic
	bind_irq(KB_IRQ_NUM,KB_ID,kb_handler,KD_POLICY);
	return;
}

DEFINE_DRIVER_REMOVE(kb) {
	//rm handler from pic
	unbind_irq(KB_IRQ_NUM,KB_ID);
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
int kb_handler(int irq, unsigned int saved_reg){
	uint8_t keyboard_scancode;

 	keyboard_scancode = inb(KB_PORT);//read the input

    //!!! WARNING: We should not use "keyboard_scancode & 0x80" anymore
    //!!!   Because that filters out the events for release keys
    //!!!   BUT we need to know when keys like SHIFT, ALT are released

 	if (!(keyboard_scancode & 0x80)) {
 		uint32_t kernel_keycode = KBascii[keyboard_scancode];
 		kb_to_term(kernel_keycode);
 	}

    return 0;
}
