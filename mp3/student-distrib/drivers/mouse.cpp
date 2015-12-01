#include <stdint.h>
#include <stddef.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/drivers/mouse.h>
#include <inc/d2d/k2m.h>
#include <inc/drivers/kbterm.h>

using namespace KeyB;
using namespace Term;


#define KD_POLICY 0
#define MOUSE_PORT 0x60
#define MOUSE_IRQ_NUM 12
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

extern "C" int mouse_handler(int irq, unsigned int saved_reg);

click_handler left_click_handler[MAX_HANDLERS];
click_handler right_click_handler[MAX_HANDLERS];

uint8_t mouse_enable_scancode;

bool last_read;
//  true for  ReadSuccess
//  false for ReadFailure

void write_byte(uint8_t data, uint8_t port);
uint8_t read_byte();
uint8_t  try_read_byte();
uint8_t try_read_byte_afterkb();
void send_command(uint8_t command, uint8_t port);
void init_mouse();
int mouse_handler();

void (*moveMouse)(int, int) = nullptr;

void send_command(uint8_t command, uint8_t port) {
    write_byte(0xD4, MOUSE_ENABLE_PORT);
    write_byte(command, port);

    for (;;)
    {
        auto a = read_byte();
        if (a == 0xFA || a == 0xAA) break;
    }
}

void write_byte(uint8_t data, uint8_t port) {
    while( (inb(MOUSE_ENABLE_PORT) & 0x2) != 0 );
    outb(data, port);
}

uint8_t read_byte() {
    while( (inb(MOUSE_ENABLE_PORT) & 0x1) == 0 );
    last_read = true;
    return inb(MOUSE_PORT);
}

uint8_t try_read_byte() {
    if ( (inb(MOUSE_ENABLE_PORT) & 0x1) == 0 ) {
        last_read = false;
        return 0;
    } else {
        last_read = true;
        return inb(MOUSE_PORT);
    }
}

DEFINE_DRIVER_INIT(mouse) {

    AutoSpinLock l(&KeyB::keyboard_lock);

    moveMouse = nullptr;

    // bind handler to pic
    init_mouse();

    bind_irq(MOUSE_IRQ_NUM,MOUSE_ID,mouse_handler,KD_POLICY);

    return;
}

DEFINE_DRIVER_REMOVE(mouse) {

    AutoSpinLock l(&KeyB::keyboard_lock);

    //rm handler from pic
    unbind_irq(MOUSE_IRQ_NUM,MOUSE_ID);

    return;
}

void init_mouse() {
    send_command(0xFF, MOUSE_PORT);//reset
    // send "Get Compaq Status Byte" command
    write_byte(0x20, MOUSE_ENABLE_PORT);
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

void registerMouseMovementHandler(void (*fn) (int, int))
{
    moveMouse = fn;
}

int mouse_handler(int irq, unsigned int saved_reg) {
    AutoSpinLock(&KeyB::keyboard_lock);

    uint8_t flags = try_read_byte();
    if (last_read == true) {
        if (flags == 0xFA) {
            // ack
            return 0;
        } else {
            if ( (flags & MOVEMENT_ONE) != 0 &&
                    (flags & X_OVERFLOW) == 0 &&
                    (flags & Y_OVERFLOW) == 0)
            {
                int32_t deltax = read_byte();
                if (flags & X_SIGN) {
                    deltax = deltax | 0xFFFFFF00;
                }
                int32_t deltay = read_byte();
                if (flags & Y_SIGN) {
                    deltay = deltay | 0xFFFFFF00;
                }
                if (moveMouse) moveMouse(deltax, deltay);

                if (flags & LEFT_BUTTON) {
                    printf("LEFT_BUTTON");
                    int i;
                    for (i = 0; i < MAX_HANDLERS; i++) {
                        if (left_click_handler[i] != NULL) {
                            //left_click_handler[i]();
                        }
                    }
                }
                if (flags & RIGHT_BUTTON) {
                    printf("RIGHT_BUTTON");
                    int i;
                    for (i = 0; i < MAX_HANDLERS; i++) {
                        if (right_click_handler[i] != NULL) {
                            //right_click_handler[i]();
                        }
                    }
                }
                return 0;

            }
        }
    }
    return 0;
}
