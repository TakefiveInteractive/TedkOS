#include <inc/drivers/rtc.h>
#include <inc/klibs/spinlock.h>
#include <inc/fs/fops.h>
#include <inc/fs/dev_wrapper.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/klibs/deque.h>

namespace rtc {

class VirtualRTC;

uint8_t frequency_converter(int freq);
void rtc_change_frequency(uint8_t rate);
void rtc_change_frequency_nolock(uint8_t rate);
void ensureHardwareFrequencyHigherThanVirtualFrequencies(VirtualRTC *requester);
void removeFromVirtualRTCs(VirtualRTC *dev);
void addToVirtualRTCs(VirtualRTC *dev);
spinlock_t rtc_lock = SPINLOCK_UNLOCKED;

Deque<VirtualRTC *> *virtualRTCs;
int hardwareRTCFrequency = 0;

class VirtualRTC : IFOps {
private:
    uint32_t counter;
    uint32_t numCountToTriggerRTC;
    thread_kinfo *thread;
    bool waiting;
    int ourFreq;
    spinlock_t _lock = SPINLOCK_UNLOCKED;

    int backlog = 0;

public:
    void triggerIfPossible()
    {
        AutoSpinLock lock(&_lock);
        if (counter >= numCountToTriggerRTC)
        {
            if (waiting)
            {
                waiting = false;
                // Trigger task
                getRegs(thread)->eax = 0;
                scheduler::unblock(thread);
            }
            else
            {
                backlog++;
            }
            counter = 0;
        }
        counter += 1;
    }

    void newHardwareRTCFrequencyNoLock(int freq)
    {
        if (freq > ourFreq)
        {
            // Scale counters
            if (ourFreq == 0) return;
            int factor = (freq / ourFreq);
            uint32_t oldNumCount = numCountToTriggerRTC;
            numCountToTriggerRTC = factor;
            if (numCountToTriggerRTC >= oldNumCount)
            {
                if (numCountToTriggerRTC != 0 && oldNumCount != 0)
                    counter *= (numCountToTriggerRTC / oldNumCount);
            }
            else
            {
                if (numCountToTriggerRTC != 0 && oldNumCount != 0)
                    counter /= (oldNumCount / numCountToTriggerRTC);
            }
        }
        else
        {
            numCountToTriggerRTC = 1;
        }
    }

    void newHardwareRTCFrequency(int freq)
    {
        AutoSpinLock lock(&_lock);
        newHardwareRTCFrequencyNoLock(freq);
    }

    int getFreq() const
    {
        return ourFreq;
    }

    /**
     * read() would block user program until rtc interrupt finished
     * @param  fd     [File descriptor]
     * @param  buf    [Buffer]
     * @param  nbytes [max number of bytes to read]
     * @return        [number of bytes read]
     */
    virtual int32_t read(FsSpecificData *fdData, uint8_t *buf, int32_t bytes)
    {
        AutoSpinLock lock(&_lock);
        if (backlog > 0)
        {
            backlog -= 1;
            return 0;
        }
        else
        {
            // wait for RTC
            auto t = getCurrentThreadInfo();
            waiting = true;
            thread = t;
            scheduler::block(t);
            return 0;
        }
    }

    /**
     * write() would change the frequency of the rtc interrupt generate
     * @param  fd     [description]
     * @param  buf    buf[0] has the frequency number
     * @param  nbytes [description]
     * @return        0 for success, -1 for frequency out of range
     */
    virtual int32_t write(FsSpecificData *fdData, const uint8_t *buf, int32_t bytes)
    {
        AutoSpinLock lock(&_lock);
        int freq = *(int *)buf;
        int rate = frequency_converter(freq);
        if (!rate) return -1;

        ourFreq = freq;
        ensureHardwareFrequencyHigherThanVirtualFrequencies(this);
        return 0;
    }

    /* close */
    virtual ~VirtualRTC()
    {
        removeFromVirtualRTCs(this);
    }

private:
    /**
     * Would initialize the rtc rate to 2Hz
     */
    VirtualRTC() : counter(0), numCountToTriggerRTC(2), waiting(false)
    {
        addToVirtualRTCs(this);
        /* open would initialize rtc frequency to 2Hz */
        int freq = 2;
        auto rate = frequency_converter(freq);
        write(nullptr, &rate, sizeof(rate));
    }

public:
    static Maybe<IFOps *> getNewInstance()
    {
        return Maybe<IFOps *>(reinterpret_cast<IFOps *>(new VirtualRTC()));
    }
};

/**
 * init() is used to initialize rtc
 */
void init()
{
    AutoSpinLock l(&rtc_lock);
    virtualRTCs = new Deque<VirtualRTC *>();

    outb(RTC_STATUS_B_NMI, RTC_ADDRESS);
    uint8_t prev = inb(RTC_DATA);
    outb(RTC_STATUS_B_NMI, RTC_ADDRESS);
    outb(prev | RTC_STATUS_B_EN, RTC_DATA);
}

void ensureHardwareFrequencyHigherThanVirtualFrequenciesNoLock(VirtualRTC *requester)
{
    // Super simple LCM for 2's power
    int max = 0;
    for (size_t i = 0; i < virtualRTCs->size(); i++)
    {
        auto freq = (*virtualRTCs)[i]->getFreq();
        if (freq > max) max = freq;
    }
    if (max != hardwareRTCFrequency)
    {
        hardwareRTCFrequency = max;
        rtc_change_frequency_nolock(frequency_converter(max));

        for (size_t i = 0; i < virtualRTCs->size(); i++)
        {
            auto d = (*virtualRTCs)[i];
            if (d == requester)
                d->newHardwareRTCFrequencyNoLock(hardwareRTCFrequency);
            else
                d->newHardwareRTCFrequency(hardwareRTCFrequency);
        }
    }
    else
    {
        if (requester != nullptr)
            requester->newHardwareRTCFrequencyNoLock(hardwareRTCFrequency);
    }
}

void removeFromVirtualRTCs(VirtualRTC *dev)
{
    AutoSpinLock lock(&rtc_lock);
    while (*(virtualRTCs->front()) != dev)
    {
        auto tmp = *(virtualRTCs->front());
        virtualRTCs->pop_front();
        virtualRTCs->push_back(tmp);
    }
    virtualRTCs->pop_front();
    ensureHardwareFrequencyHigherThanVirtualFrequenciesNoLock(nullptr);
}

void addToVirtualRTCs(VirtualRTC *dev)
{
    AutoSpinLock lock(&rtc_lock);
    virtualRTCs->push_back(dev);
    ensureHardwareFrequencyHigherThanVirtualFrequenciesNoLock(dev);
}

void ensureHardwareFrequencyHigherThanVirtualFrequencies(VirtualRTC *requester)
{
    AutoSpinLock lock(&rtc_lock);
    ensureHardwareFrequencyHigherThanVirtualFrequenciesNoLock(requester);
}

/**
 * handler() is interrupt handler for rtc
 * @param  irq       interrupt number
 * @param  saved_reg not used for now
 * @return           0 for success, always 0
 */
int handler(int irq, unsigned int saved_reg)
{
    AutoSpinLock lock(&rtc_lock);
    /* test_interrupts(); */
    /* read register c to allow future use */
    outb(RTC_STATUS_C, RTC_ADDRESS);
    inb(RTC_DATA);

    for (size_t i = 0; i < virtualRTCs->size(); i++)
    {
        (*virtualRTCs)[i]->triggerIfPossible();
    }

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

void rtc_change_frequency_nolock(uint8_t rate)
{
    if (rate == 0)
        return;
    outb(RTC_STATUS_A_NMI, RTC_ADDRESS);
    uint8_t prev = inb(RTC_DATA);
    outb(RTC_STATUS_A_NMI, RTC_ADDRESS);
    outb((prev & HIGH_BIT_MASK) | rate, RTC_DATA);
}

/**
 * private helper function for write() that changes the rate of rtc
 * @param rate - rtc understandable binary, range from 0x02 to 0x0F
 */
void rtc_change_frequency(uint8_t rate)
{
    AutoSpinLock lock(&rtc_lock);
    rtc_change_frequency_nolock(rate);
}

}

DEFINE_DRIVER_INIT(rtc)
{
    bind_irq(RTC_IRQ, RTC_ID, rtc::handler, RTC_POLICY);
    rtc::init();
    filesystem::register_devfs("rtc", []() { return rtc::VirtualRTC::getNewInstance(); });
    return;
}

DEFINE_DRIVER_REMOVE(rtc)
{
    unbind_irq(RTC_IRQ, RTC_ID);
    return;
}

