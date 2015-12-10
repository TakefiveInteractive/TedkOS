#include <inc/drivers/scan_pci.h>
#include <inc/drivers/pci.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/AutoSpinLock.h>

Deque<pci::UnitDev> *PATAControllers = NULL;
spinlock_t scanpciLock = SPINLOCK_UNLOCKED;
using namespace pci;

DEFINE_DRIVER_INIT(scanpci)
{
    AutoSpinLock l(&scanpciLock);

    PATAControllers = new Deque<UnitDev>();

    printf("Scanning PCI devices...\n");

    findAllFunctions([](uint8_t bus, uint8_t device, uint8_t func)
    {
        uint32_t basicIDs = Register(bus, device, func, 0).get();
        uint32_t vendorID = basicIDs & 0xffff;
        uint32_t deviceID = basicIDs >> 16;

        printf("Vendor 0x%x Device 0x%x Func 0x%x ", vendorID, deviceID, func);

        uint32_t classInfo = Register(bus, device, func, 0x8).get();
        if((classInfo >> 16) == 0x0101)
        {
            printf("\t DISK!");
            PATAControllers->push_back(UnitDev(bus, device, func));
            Register intConfig(bus, device, func, 0x3C);
            //intConfig.set((intConfig.get() & 0xFFFFFF00) | 14);
        }
        else printf("\tclass = 0x%x", classInfo >> 24);

        printf(" subclass = 0x%x\n", (classInfo >> 16) & 0xff);

    });
}

DEFINE_DRIVER_REMOVE(scanpci)
{
    AutoSpinLock l(&scanpciLock);
    
    if(PATAControllers)
        PATAControllers->clear();
}

