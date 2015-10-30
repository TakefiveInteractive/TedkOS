#ifndef _RTC_DRIVER_H
#define _RTC_DRIVER_H

#include <inc/driver.h>

#include "lib.h"
#include <stddef.h>
#include "i8259.h"

/** RTC IRQ */
#define RTC_IRQ 8
/** RTC device id */
#define RTC_ID 8
/** RTC policy id */
#define RTC_POLICY 0
/** RTC address (and NMI) register */
#define RTC_ADDRESS 0x70
/** RTC data register */
#define RTC_DATA 0x71
/** RTC status register A */
#define RTC_STATUS_A 0x0A
/** RTC status register B */
#define RTC_STATUS_B 0x0B
/** RTC status register C */
#define RTC_STATUS_C 0x0C
/** RTC status register D */
#define RTC_STATUS_D 0x0D
/** RTC non-mask status register A */
#define RTC_STATUS_A_NMI 0x8A
/** RTC non-mask status register B */
#define RTC_STATUS_B_NMI 0x8B
/** RTC non-mask status register C */
#define RTC_STATUS_C_NMI 0x8C
/** RTC non-mask status register D */
#define RTC_STATUS_D_NMI 0x8D
/** RTC Periodic Interrupt Enabled bit */
#define RTC_STATUS_B_EN 0x40
/** High bit */
#define HIGH_BIT_MASK 0xF0

DEFINE_DRIVER_INIT(rtc);
DEFINE_DRIVER_REMOVE(rtc);

void rtc_init();
int rtc_handler(int irq, unsigned int saved_reg);

#endif
