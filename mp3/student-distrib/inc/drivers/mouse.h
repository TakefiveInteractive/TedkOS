#ifndef _MOUSE_DRIVER_H
#define _MOUSE_DRIVER_H


#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/common.h>
#include <inc/drivers/pit.h>

#ifdef __cplusplus

#include <inc/klibs/lib.h>
#include <inc/i8259.h>
#include <inc/klibs/AutoSpinLock.h>

void registerMouseMovementHandler(void (*fn) (int, int));

namespace ui {

void leftClickHandler();
void leftReleaseHandler();
void dragHandler();

}

#endif//#ifdef __cplusplus

DEFINE_DRIVER_INIT(mouse);
DEFINE_DRIVER_REMOVE(mouse);


#endif//_MOUSE_DRIVER_H
