#ifndef _RTC_DRIVER_H
#define _RTC_DRIVER_H

#include <inc/driver.h>

#include "lib.h"
#include <stddef.h>
#include "i8259.h"

extern DEFINE_DRIVER_INIT(rtc);
extern DEFINE_DRIVER_REMOVE(rtc);

#endif
