#include <inc/keyboard.h>
//#include "keyboard.h"

#define KB_PORT 0x21
#define KB_IRQ 1
#define KB_ID 1
#define KD_POLICY 0

unsigned char KBascii[128] =
{
    0,  27,
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'-', '=',
	'\b',/* Backspace */
	'\t',/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']',
	'\n',	/* Enter key */
    0,	/* 29   - Control */
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


DEFINE_DRIVER_INIT(kb) {
	//enable_irq(KB_IRQ);
	bind_irq(KB_IRQ,KB_ID,kb_handler,KD_POLICY);
	//bind handler to pic
	return;
}

DEFINE_DRIVER_REMOVE(kb) {
	//rm handler from pic
	unbind_irq(KB_IRQ,KB_ID);
	//disable_irq(KB_IRQ);
    return;
}

/* keyboard_handler
 * description:
 *		initialize the keyboard
 * input,output,return:
 *		none
 * side effect:
 *		initialize the keyboard
 */
void kb_handler(int irq, pt_reg* saved_reg){
	
	return;
}
