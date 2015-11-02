#include <inc/rtc.h>
#include <inc/klibs/spinlock.h>

uint8_t frequency_converter(int freq);
void rtc_change_frequency(uint8_t rate);
int interrupt_flag = 0;
spinlock_t lock = SPINLOCK_UNLOCKED;

DEFINE_DRIVER_INIT(rtc)
{
    bind_irq(RTC_IRQ, RTC_ID, rtc_handler, RTC_POLICY);
    rtc_init();
	return;
}

DEFINE_DRIVER_REMOVE(rtc)
{
    unbind_irq(RTC_IRQ, RTC_ID);
    return;
}

/**
 * rtc_init() is used to initialize rtc
 */
void rtc_init()
{
    outb(RTC_STATUS_B_NMI, RTC_ADDRESS);
    uint8_t prev = inb(RTC_DATA);
    outb(RTC_STATUS_B_NMI, RTC_ADDRESS);
    outb(prev | RTC_STATUS_B_EN, RTC_DATA);
}

/**
 * rtc_handler() is interrupt handler for rtc
 * @param  irq       interrupt number
 * @param  saved_reg not used for now
 * @return           0 for success, always 0
 */
int rtc_handler(int irq, unsigned int saved_reg)
{
    unsigned int flag;
    spin_lock_irqsave(&lock, flag);
    /* test_interrupts(); */
    /* read register c to allow future use */
    outb(RTC_STATUS_C, RTC_ADDRESS);
    inb(RTC_DATA);
    interrupt_flag = 1;
    spin_unlock_irqrestore(&lock, flag);
    return 0;
}

/**
 * read() would block user program until rtc interrupt finished
 * @param  fd     [description]
 * @param  buf    [description]
 * @param  nbytes [description]
 * @return        [description]
 */
int rtc_read (int fd, void* buf, int nbytes)
{
    // wait until interrupt_flag sets to 1
    while (interrupt_flag == 0) {;}
    // set interrupt flag back to 0
    interrupt_flag = 0;
    return 0;
}

/**
 * write() would change the frequency of the rtc interrupt generate
 * @param  fd     [description]
 * @param  buf    buf[0] has the frequency number
 * @param  nbytes [description]
 * @return        0 for success, -1 for frequency out of range
 */
int rtc_write (int fd, const void* buf, int nbytes)
{
    int freq = *(int *)buf;
    int rate = frequency_converter(freq);
    if (!rate) return -1;
    rtc_change_frequency(rate);
    return 0;
}

/**
 * open() would initialize the rtc rate to 2Hz
 * @param  filename [description]
 * @return          return 0 always
 */
int rtc_open (const uint8_t* filename)
{
    /* open would initialize rtc frequency to 2Hz */
    uint8_t rate = frequency_converter(2);
    rtc_change_frequency(rate);
    return 0;
}

/**
 * close() did nothing, as TA mentioned
 * @param  fd [description]
 * @return    always return 0
 */
int rtc_close (int fd)
{
    return 0;
}

/**
 * private function that convert user input frequency to rtc binary
 * @param  freq - frequency of the rtc, e.g. 2 for 2Hz
 * @return      rtc understandable binary presentation of actual frequency
 */
uint8_t frequency_converter(int freq)
{
    // rate must be above 0x02 and not over 0x0F
    switch (freq) {
        // case 16384: return 0x02;
        // case 8192: return 0x03;
        // case 4096: return 0x04;
        // case 2048: return 0x05;
        case 1024: return 0x06;
        case 512: return 0x07;
        case 256: return 0x08;
        case 128: return 0x09;
        case 64: return 0x0A;
        case 32: return 0x0B;
        case 16: return 0x0C;
        case 8: return 0x0D;
        case 4: return 0x0E;
        case 2: return 0x0F;
        default: return 0;
    }
}

/**
 * private helper function for write() that changes the rate of rtc
 * @param rate - rtc understandable binary, range from 0x02 to 0x0F
 */
void rtc_change_frequency(uint8_t rate)
{
    unsigned int flag;
    spin_lock_irqsave(&lock, flag);
    outb(RTC_STATUS_A_NMI, RTC_ADDRESS);
    uint8_t prev = inb(RTC_DATA);
    outb(RTC_STATUS_A_NMI, RTC_ADDRESS);
    outb((prev & HIGH_BIT_MASK) | rate, RTC_DATA);
    spin_unlock_irqrestore(&lock, flag);
}
