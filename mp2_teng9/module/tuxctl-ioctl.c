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
volatile unsigned int led_saved_arg;
volatile unsigned char button_b;
volatile unsigned char button_c;
static const char CMD_MTCP_BIOC_ON = MTCP_BIOC_ON;
static const char CMD_MTCP_LED_USR = MTCP_LED_USR;

/* Font for digits 0 to f */
static unsigned char led_font[16] = {
     0xE7, 0x06, 0xCB, 0x8F,
     0x2E, 0xAD, 0xED, 0x86,
     0xEF, 0xAE, 0xEE, 0x6D,
     0xE1, 0x4F, 0xE9, 0xE8
};

int tuxctl_set_led(struct tty_struct* tty, unsigned long arg);
int tuxctl_set_button(unsigned long* arg);

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in
 * tuxctl-ld.c. It calls this function, so all warnings there apply
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    static spinlock_t lock = SPIN_LOCK_UNLOCKED;
    uint32_t flags;
    spin_lock_irqsave(&lock, flags);

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    switch (a) {
        case MTCP_RESET:
            transmitting = 0;
            tuxctl_ldisc_put(tty, &CMD_MTCP_BIOC_ON, 1);
            tuxctl_set_led(tty, led_saved_arg);
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
    int ptr;

    if (transmitting) return -EINVAL;

    switch (cmd)
    {
    case TUX_INIT:
        tuxctl_ldisc_put(tty, &CMD_MTCP_BIOC_ON, 1);
        tuxctl_ldisc_put(tty, &CMD_MTCP_LED_USR, 1);
        return 0;

    case TUX_BUTTONS:
        if (!arg) return -EINVAL;

        ptr = 0x0;
        ptr |= (button_b & 0x0F);
        ptr |= (button_c & 0x0F) << 4;

        ret = copy_to_user((int*)arg, &ptr, sizeof(int));
        if (ret > 0)
            return -EINVAL;
        else
            return 0;

    case TUX_SET_LED:
        return tuxctl_set_led(tty, arg);

    case TUX_LED_ACK:
    case TUX_LED_REQUEST:
    case TUX_READ_LED:
    default:
        return -EINVAL;
    }

}

int tuxctl_set_button(unsigned long* arg)
{
    unsigned char low = button_b & 0x0F;
    unsigned char high = button_c & 0x0F;
    unsigned char mixed = 0x0;
    unsigned char direction;
    int i;
    int mask = 0x01;
    unsigned long lll;
    for (i = 0; i < 4; i++) {
        direction = high & mask;
        mixed |= direction;
        mask = mask << 1;
    }

    mixed = (high << 4) | low;
    lll = (unsigned long)mixed;
    return copy_to_user(arg, &lll, sizeof(long));
}

int tuxctl_set_led(struct tty_struct* tty, unsigned long arg)
{
    int i;
    int led_dot = 0x01000000;
    int led_on = 0x00010000;
    unsigned char led_text;
    unsigned char time_buf[4];
    unsigned char led_buf[LED_BUF_SIZE];

    led_buf[0] = MTCP_LED_SET;
    led_buf[1] = 0x0F;
    time_buf[0] = (arg & 0x000F);
    time_buf[1] = (arg & 0x00F0) >> 4;
    time_buf[2] = (arg & 0x0F00) >> (4 * 2);
    time_buf[3] = (arg & 0x0F000) >> (4 * 3);

    transmitting = 1;
    for (i = 0; i < LED_SCREEN_NUM; i++) {
        if (arg & led_on) {
            led_text = led_font[time_buf[i]];
            if (arg & led_dot)
                led_text |= 0x10;
        } else led_text = 0x0;
        led_buf[i + 2] = led_text;

        led_on = led_on << 1;
        led_dot = led_dot << 1;
    }
    led_saved_arg = arg;
    tuxctl_ldisc_put(tty, (char*)led_buf, LED_BUF_SIZE);
    return 0;
}

