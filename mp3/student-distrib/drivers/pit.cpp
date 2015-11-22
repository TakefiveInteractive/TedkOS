#include <inc/drivers/pit.h>

volatile int pit_interrupt_occurred = 0;
spinlock_t pit_lock = SPINLOCK_UNLOCKED;

int PIT_DATA_PORT(int channel) { return 0x40 + channel; }

/**
 * interface to configure PITj
 * @param  channel 0 - sytem timer
 *                 2 - speaker
 * @param  mode    2 - rate generator
 *                 3 - square wave
 * @param  freq    has to between 19 and 1193182
 * @return         return -1 if error occured
 */
int pit_config(int channel, int mode, int freq)
{
    int16_t rate = 0;
    int16_t data = 0;

    // limiting to forseably usable channels/modes
    if (channel != 0 && channel != 2) return -1;
    if (mode != 2 && mode != 3) return -1;

    rate = (freq < PIT_MIN_FREQ) ? 0
        : (freq > PIT_MAX_FREQ) ? 2
        : (PIT_MAX_FREQ + freq / 2) / freq;

    data = (channel << 6) | 0x30 | (mode << 1);

    // NMI_disable();
    outb(data, PIT_CMD_PORT);
    outb(rate, PIT_DATA_PORT(channel));
    outb(rate >> 8, PIT_DATA_PORT(channel));
    // NMI_enable();

    return 0;
}

int pit_init(int freq)
{
    return pit_config(0, 2, freq);
}

int32_t pit_read (void* fd, uint8_t* buf, int32_t nbytes)
{
    unsigned int flag;
    spin_lock_irqsave(&pit_lock, flag);
    pit_interrupt_occurred = 0;
    spin_unlock_irqrestore(&pit_lock, flag);

    while (!pit_interrupt_occurred);
    return 0;
}

int pit_handler(int irq, unsigned int saved_reg)
{
    AutoSpinLock lock(&pit_lock);
    //TODO need to call scheduler here
    pit_interrupt_occurred = 1;
    return 0;
}

DEFINE_DRIVER_INIT(pit)
{
    bind_irq(PIT_IRQ, PIT_ID, pit_handler, PIT_POLICY);
    // pit_init function goes here if applicable
    return;
}

DEFINE_DRIVER_REMOVE(pit)
{
    unbind_irq(PIT_IRQ, PIT_ID);
    return;
}
