/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
    printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

#define LED_BUF_SIZE 6
#define LED_SCREEN_NUM 4

volatile int transmitting = 0;
volatile unsigned char button_b = 0xff;
volatile unsigned char button_c = 0xff;
static const char CMD_MTCP_BIOC_ON = MTCP_BIOC_ON;
static const char CMD_MTCP_LED_USR = MTCP_LED_USR;

// Saves text in LED
volatile unsigned int prev_led_state;

int tuxctl_set_led(struct tty_struct* tty, unsigned long arg);
uint8_t tuxctl_translate_button(void);

static spinlock_t lock = SPIN_LOCK_UNLOCKED;

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in
 * tuxctl-ld.c. It calls this function, so all warnings there apply
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    uint32_t flags;
    spin_lock_irqsave(&lock, flags);

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    switch (a) {
        case MTCP_RESET:
            transmitting = 0;
            // Restore Tux controller states
            // Interrupt-On-Change
            tuxctl_ldisc_put(tty, &CMD_MTCP_BIOC_ON, 1);
            // LED text
            tuxctl_set_led(tty, prev_led_state);
            // Clear button state
            button_b = 0xff;
            button_c = 0xff;
            transmitting = 1;
            break;
        case MTCP_ACK:
            transmitting = 0;
            break;
        case MTCP_BIOC_EVENT:
            button_b = b;
            button_c = c;
            break;
        default:
            printk("unknown packet");
    }

    /*printk("packet : %x %x %x\n", a, b, c); */
    spin_unlock_irqrestore(&lock, flags);
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int
tuxctl_ioctl (struct tty_struct* tty, struct file* file,
          unsigned cmd, unsigned long arg)
{
    int ret;
    int res;
    uint32_t flags;

    switch (cmd)
    {
    case TUX_INIT:
        // Clear button state
        button_b = 0xff;
        button_c = 0xff;
        // Clear LED
        prev_led_state = 0;

        if (transmitting) return -EINVAL;

        spin_lock_irqsave(&lock, flags);
        tuxctl_ldisc_put(tty, &CMD_MTCP_BIOC_ON, 1);
        tuxctl_ldisc_put(tty, &CMD_MTCP_LED_USR, 1);
        spin_unlock_irqrestore(&lock, flags);

        return 0;

    case TUX_BUTTONS:
        if (!arg) return -EINVAL;

        res = tuxctl_translate_button();
        ret = copy_to_user((int*) arg, &res, sizeof(int));
        if (ret > 0)
            return -EINVAL;
        else
            return 0;

    case TUX_SET_LED:
        if (transmitting) return -EINVAL;
        return tuxctl_set_led(tty, arg);

    case TUX_LED_ACK:
    case TUX_LED_REQUEST:
    case TUX_READ_LED:
    default:
        return -EINVAL;
    }

}

// Sets a particular bit in a uint8
static inline void tux_set_bit(uint8_t *x, int bitNum, int val) {
    if (val)
        *x |= (1 << bitNum);
    else
        *x &= ~(1 << bitNum);
}

// Gets a particular bit in a uint8
static inline int tux_get_bit(uint8_t x, int bitNum) {
    return (x & (1 << bitNum)) != 0;
}

// Translate MTCP button status to ioctl result format
uint8_t tuxctl_translate_button()
{
    uint8_t button2 = ~button_b;
    uint8_t button3 = ~button_c;

    char right = tux_get_bit(button3, 3);
    char left = tux_get_bit(button3, 1);
    char down = tux_get_bit(button3, 2);
    char up = tux_get_bit(button3, 0);
    char c = tux_get_bit(button2, 3);
    char b = tux_get_bit(button2, 2);
    char a = tux_get_bit(button2, 1);
    char start = tux_get_bit(button2, 0);

    uint8_t res = 0;
    // These are directly from the Tux spec
    tux_set_bit(&res, 7, right);
    tux_set_bit(&res, 6, left);
    tux_set_bit(&res, 5, down);
    tux_set_bit(&res, 4, up);
    tux_set_bit(&res, 3, c);
    tux_set_bit(&res, 2, b);
    tux_set_bit(&res, 1, a);
    tux_set_bit(&res, 0, start);
    return res;
}

/* Font for digits 0 to f */
static unsigned char led_font[16] = {
     0xE7, 0x06, 0xCB, 0x8F,
     0x2E, 0xAD, 0xED, 0x86,
     0xEF, 0xAE, 0xEE, 0x6D,
     0xE1, 0x4F, 0xE9, 0xE8
};

int tuxctl_set_led(struct tty_struct* tty, unsigned long arg)
{
    int i;
    uint32_t flags;

    int led_dot = 0x01000000;
    int led_on = 0x00010000;
    unsigned char time_buf[4];
    unsigned char command[LED_BUF_SIZE];

    command[0] = MTCP_LED_SET;
    command[1] = 0x0F;

    time_buf[0] = (arg & 0x000F);
    time_buf[1] = (arg & 0x00F0) >> 4;
    time_buf[2] = (arg & 0x0F00) >> 8;
    time_buf[3] = (arg & 0xF000) >> 12;

    transmitting = 1;
    for (i = 0; i < LED_SCREEN_NUM; i++)
    {
        unsigned char led_text;
        if (arg & led_on)
        {
            led_text = led_font[time_buf[i]];
            if (arg & led_dot) led_text |= 0x10;
        }
        else
        {
            led_text = 0x0;
        }
        command[i + 2] = led_text;

        led_on = led_on << 1;
        led_dot = led_dot << 1;
    }
    prev_led_state = arg;

    spin_lock_irqsave(&lock, flags);
    tuxctl_ldisc_put(tty, (char*) command, LED_BUF_SIZE);
    spin_unlock_irqrestore(&lock, flags);

    return 0;
}

