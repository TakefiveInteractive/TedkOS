#ifndef _MOUSE_DRIVER_H
#define _MOUSE_DRIVER_H


#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/common.h>

#ifdef __cplusplus

#include <inc/klibs/lib.h>
#include <inc/i8259.h>
#include <inc/klibs/AutoSpinLock.h>

void registerMouseMovementHandler(void (*fn) (int, int));

#endif//#ifdef __cplusplus

DEFINE_DRIVER_INIT(mouse);
DEFINE_DRIVER_REMOVE(mouse);


#endif//_MOUSE_DRIVER_H
