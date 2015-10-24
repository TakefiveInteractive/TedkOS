#include <inc/rtc.h>

DEFINE_DRIVER_INIT(rtc)
{
    bind_irq(RTC_IRQ, RTC_ID, (irq_good_handler_t)test_interrupts, RTC_POLICY);
	return;
}

DEFINE_DRIVER_REMOVE(rtc)
{
    unbind_irq(RTC_IRQ, RTC_ID);
    return;
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
    return 0;
}