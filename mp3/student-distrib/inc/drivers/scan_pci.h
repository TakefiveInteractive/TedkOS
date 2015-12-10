#ifndef _DRIVER_SCANPCI_H
#define _DRIVER_SCANPCI_H

#include <inc/drivers/common.h>
#include <inc/klibs/deque.h>
#include <inc/drivers/pci.h>

// IDE <=> PATA
extern Deque<pci::UnitDev> *PATAControllers;

extern DEFINE_DRIVER_INIT(scanpci);
extern DEFINE_DRIVER_REMOVE(scanpci);

#endif//_DRIVER_SCANPCI_H
