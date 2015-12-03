#include <inc/drivers/pit.h>
#include <inc/x86/idt_init.h>
#include <inc/proc/sched.h>

spinlock_t pit_lock = SPINLOCK_UNLOCKED;
volatile int32_t tick_counter = 0;//counter will inc 1 every time when irq
uint32_t freq_copy = 0;

int PIT_DATA_PORT(int channel) { return 0x40 + channel; }

/**
 * interface to configure PIT
 * @param  channel 0 - sytem timer
 *                 2 - speaker
 * @param  mode    2 - rate generator
 *                 3 - square wave
 * @param  freq    has to between 19 and 1193182
 * @return         return -1 if error occured
 */
int pit_config(int channel, int mode, uint32_t freq)
{
    AutoSpinLock lock(&pit_lock);

    freq_copy = freq;

    int16_t rate = 0;
    int16_t data = 0;

    // limiting to forseably usable channels/modes
    if (channel != 0 && channel != 2) return -1;
    if (mode != 2 && mode != 3) return -1;

    rate = (freq < PIT_MIN_FREQ) ? 0
        : (freq > PIT_MAX_FREQ) ? 2
        : (PIT_MAX_FREQ + freq / 2) / freq;

    data = (channel << 6) | 0x30 | (mode << 1);

    runWithoutNMI([data, rate, channel] () {
        outb(data, PIT_CMD_PORT);
        outb(rate, PIT_DATA_PORT(channel));
        outb(rate >> 8, PIT_DATA_PORT(channel));
    });

    return 0;
}

int pit_init(uint32_t freq)
{
    return pit_config(0, 2, freq);
}

int pit_handler(int irq, unsigned int saved_reg)
{
    AutoSpinLock lock(&pit_lock);
    tick_counter++;
    scheduler::makeDecision();
    return 0;
}

uint32_t pit_gettick()
{
    return tick_counter;
}
uint32_t pit_tick2time(uint32_t tick)
{
    return (uint32_t)(tick*(1e9/freq_copy));
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
