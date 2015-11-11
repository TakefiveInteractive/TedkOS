#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/spinlock.h>
#include <inc/mouse.h>


#define KD_POLICY 0
#define MOUSE_PORT 0x60
#define MOUSE_INT_NUM 0x21
#define MOUSE_IRQ_NUM 1
#define KB_ID 1


/*
 * See: http://wiki.osdev.org/Mouse_Input
 */
void write_byte(uint8_t data, uint8_t port);
void send_command(uint8_t command, uint8_t port);

void send_command(uint8_t command, uint8_t port) {
	write_byte(0xD4, 0x64);
	write_byte(command, port);
}

void write_byte(uint8_t data, uint8_t port) {
	while( (inb(0x64) & 0x2) != 0 );
	outb(data, port);
}

spinlock_t mouse_lock = SPINLOCK_UNLOCKED;


void mouse_handler();


DEFINE_DRIVER_INIT(mouse) {
    uint32_t flag;
    spin_lock_irqsave(&mouse_lock, flag);

	// bind handler to pic
	bind_irq(MOUSE_IRQ_NUM,MOUSE_ID,mouse_handler,KD_POLICY);

    spin_unlock_irqrestore(&keyboard_lock, flag);
	return;
}

DEFINE_DRIVER_REMOVE(mouse) {
    uint32_t flag;
    spin_lock_irqsave(&mouse_lock, flag);

	//rm handler from pic
	unbind_irq(MOUSE_IRQ_NUM,MOUSE_ID);
    //pending_special = 0;

    spin_unlock_irqrestore(&mouse_lock, flag);
    return;
}
