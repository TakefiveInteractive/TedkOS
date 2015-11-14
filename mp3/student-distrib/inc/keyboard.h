#ifndef _KB_DRIVER_H
#define _KB_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <inc/driver.h>

#include <inc/klibs/lib.h>
#include <inc/i8259.h>

extern spinlock_t keyboard_lock;

DEFINE_DRIVER_INIT(kb);
DEFINE_DRIVER_REMOVE(kb);


#endif
