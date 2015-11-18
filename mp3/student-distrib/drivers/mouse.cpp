#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/spinlock.h>
#include <inc/drivers/mouse.h>
#include <inc/d2d/k2m.h>
#include <inc/drivers/keyboard.h>


#define KD_POLICY 0
#define MOUSE_PORT 0x60
#define MOUSE_INT_NUM 0x21
#define MOUSE_IRQ_NUM 1
#define MOUSE_ID 1
#define LEFT_BUTTON (1 << 0)
#define RIGHT_BUTTON (1 << 1)
#define MIDDLE_BUTTON (1 << 2)
#define MOVEMENT_ONE (1 << 3)
#define X_SIGN (1 << 4)
#define Y_SIGN (1 << 5)
#define X_OVERFLOW (1 << 6)
#define Y_OVERFLOW (1 << 7)



/*
 * See: http://wiki.osdev.org/Mouse_Input
 */

#define MAX_HANDLERS 3

typedef void (*click_handler)(int32_t, int32_t);

click_handler left_click_handler[MAX_HANDLERS];
click_handler right_click_handler[MAX_HANDLERS];

enum read_status {
     ReadSuccess,
     ReadFailure,
};
enum read_status last_read;

void write_byte(uint8_t data, uint8_t port);
uint8_t read_byte();
uint8_t  try_read_byte();
uint8_t try_read_byte_afterkb();
void send_command(uint8_t command, uint8_t port);
void init_mouse();
int mouse_handler();


void send_command(uint8_t command, uint8_t port) {
	write_byte(0xD4, MOUSE_ENABLE_PORT);
	write_byte(command, port);
}

void write_byte(uint8_t data, uint8_t port) {
	while( (inb(MOUSE_ENABLE_PORT) & 0x2) != 0 );
	outb(data, port);
}

uint8_t read_byte() {
	while( (inb(MOUSE_ENABLE_PORT) & 0x1) == 0 );
    last_read = ReadSuccess;
	return inb(MOUSE_PORT);
}

uint8_t try_read_byte() {
    if ( (inb(MOUSE_ENABLE_PORT) & 0x1) == 0 ) {
        last_read = ReadFailure;
        return 0;
    } else {
        last_read = ReadSuccess;
        return inb(MOUSE_PORT);
    }
}

uint8_t try_read_byte_afterkb() {
    if ( (mouse_enable_scancode & 0x1) == 0 ) {
        last_read = ReadFailure;
        return 0;
    } else {
        last_read = ReadSuccess;
        return inb(MOUSE_PORT);
    }
}

DEFINE_DRIVER_INIT(mouse) {
    uint32_t flag;
    spin_lock_irqsave(&keyboard_lock, flag);
	// bind handler to pic
    init_mouse();

	bind_irq(MOUSE_IRQ_NUM,MOUSE_ID,mouse_handler,KD_POLICY);
    //init_mouse();

    spin_unlock_irqrestore(&keyboard_lock, flag);
	return;
}

DEFINE_DRIVER_REMOVE(mouse) {
    uint32_t flag;
    spin_lock_irqsave(&keyboard_lock, flag);
	//rm handler from pic
	unbind_irq(MOUSE_IRQ_NUM,MOUSE_ID);

    spin_unlock_irqrestore(&keyboard_lock, flag);
    return;
}

void init_mouse() {
    send_command(0xFF, MOUSE_PORT);//rest
    // send "Get Compaq Status Byte" command
	send_command(0x20, MOUSE_ENABLE_PORT);
	uint8_t compaq_status = read_byte();
	// enable IRQ 12
	compaq_status |= 0x2;
	// clear Disable Mouse Click
	compaq_status &= ~(0x20);
    // "Send Compaq Status"
	send_command(0x60, MOUSE_ENABLE_PORT);
	write_byte(compaq_status, MOUSE_PORT);

    int i;
    for (i = 0; i < MAX_HANDLERS; i++) {
        left_click_handler[i] = NULL;
        right_click_handler[i] = NULL;
    }

	// enable acks
	send_command(0xF4, MOUSE_PORT);
}


int mouse_handler() {

    uint32_t flag;

    spin_lock_irqsave(&keyboard_lock, flag);


    if (mouse_enable_scancode != 0x20) //should be handle by ms
    {
        spin_unlock_irqrestore(&keyboard_lock, flag);
        return 0;
    }
    //init_mouse();
    uint8_t flags = try_read_byte_afterkb();
    if (last_read == ReadSuccess) {
        if (flags == 0xFA) {
            // ack
            spin_unlock_irqrestore(&keyboard_lock, flag);

            return 0;
        } else {
            if ( (flags & MOVEMENT_ONE) != 0 &&
                    (flags & X_OVERFLOW) == 0 &&
                    (flags & Y_OVERFLOW) == 0)
            {
                // int32_t deltax = read_byte();
                // if (flags & X_SIGN) {
                //     deltax = deltax | 0xFFFFFF00;
                // }
                // int32_t deltay = read_byte();
                // if (flags & Y_SIGN) {
                //     deltay = deltay | 0xFFFFFF00;
                // }
                // move_mouse(deltax, deltay);

                if (flags & LEFT_BUTTON) {
                    int i;
                    for (i = 0; i < MAX_HANDLERS; i++) {
                        if (left_click_handler[i] != NULL) {
                            printf("LEFT_BUTTON");
                            //left_click_handler[i](mouse_pos.x/X_SCALE, mouse_pos.y/Y_SCALE);
                        }
                    }
                }
                if (flags & RIGHT_BUTTON) {
                    int i;
                    for (i = 0; i < MAX_HANDLERS; i++) {
                        if (right_click_handler[i] != NULL) {
                            printf("RIGHT_BUTTON");
                            //right_click_handler[i](mouse_pos.x/X_SCALE, mouse_pos.y/Y_SCALE);
                        }
                    }
                }
                if (flags & MIDDLE_BUTTON) {
                    //not specified
                }
                spin_unlock_irqrestore(&keyboard_lock, flag);
                return 0;

            }
        }
    }
    spin_unlock_irqrestore(&keyboard_lock, flag);

    return 0;
}
