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
/** RTC Periodic Interrupt Enabled bit */
#define RTC_STATUS_B_EN 0x40

extern DEFINE_DRIVER_INIT(rtc);
extern DEFINE_DRIVER_REMOVE(rtc);

#endif
