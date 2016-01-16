#ifndef _DRIVER_ATA_PUBLIC_H
#define _DRIVER_ATA_PUBLIC_H

#include <inc/drivers/common.h>

#define ATA_DMA_ID      0xA2AD3A

extern DEFINE_DRIVER_INIT(pata);
extern DEFINE_DRIVER_REMOVE(pata);

#endif//_DRIVER_ATA_PUBLIC_H
