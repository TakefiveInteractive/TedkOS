#include <inc/rtc.h>

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

void rtc_init()
{
    cli();
    outb(RTC_STATUS_B_NMI, RTC_ADDRESS);
    uint8_t prev = inb(RTC_DATA);
    outb(RTC_STATUS_B_NMI, RTC_ADDRESS);
    outb(prev | RTC_STATUS_B_EN, RTC_DATA);
    sti();

    // rate must be above 2 and not over 15
    uint8_t rate = 0x0F;
    cli();
    outb(RTC_STATUS_A_NMI, RTC_ADDRESS);
    prev = inb(RTC_DATA);
    outb(RTC_STATUS_A_NMI, RTC_ADDRESS);
    outb((prev & HIGH_BIT_MASK) | rate, RTC_DATA);
    sti();
}

/* rtc_handler  *********** NOT IMPLEMENTING FOR NOW
 * DESCRIPTION: Handle rtc interrupts
 * INPUT: irq -- interrupt number
 *        saved_reg -- not used for now
 * OUTPUT: current the returned value is ignored, 0 for success
 *         look at <inc/error.h> for more error codes
 * SIDE EFFECT: NOT FOR NOW
 */
int rtc_handler(int irq, unsigned int saved_reg)
{
    test_interrupts();
    outb(RTC_STATUS_C, RTC_ADDRESS);
    inb(RTC_DATA);
    return 0;
}