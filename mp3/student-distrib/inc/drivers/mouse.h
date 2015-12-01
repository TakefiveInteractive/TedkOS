#ifndef _MOUSE_DRIVER_H
#define _MOUSE_DRIVER_H


#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/common.h>

#include <inc/klibs/lib.h>
#include <inc/i8259.h>

DEFINE_DRIVER_INIT(mouse);
DEFINE_DRIVER_REMOVE(mouse);

void registerMouseMovementHandler(void (*fn) (int, int));

#endif
