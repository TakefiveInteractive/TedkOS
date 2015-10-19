#ifndef _KB_DRIVER_H
#define _KB_DRIVER_H

#include <inc/driver.h>

#include "lib.h"
#include "types.h"
#include "i8259.h"

extern DEFINE_DRIVER_INIT(kb);
extern DEFINE_DRIVER_REMOVE(kb);

#endif

