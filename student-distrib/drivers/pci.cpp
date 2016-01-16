#include <inc/drivers/pci.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/lib.h>

#define PORT_PCI_CONFIG_ADDR        0xCF8
#define PORT_PCI_CONFIG_DATA        0xCFC

#define PCI_NUM_BUS                 256
#define PCI_NUM_DEVICE              32              // 32 devices per bus
#define PCI_NUM_FUNC                8               // 8 functions per device
#define PCI_NUM_REGISTER            64              // 64 registers per function

namespace pci
{

spinlock_t Register::ioLock = SPINLOCK_UNLOCKED;

Register::Register(const UnitDev& unitDev, uint8_t registerId) : Register(unitDev.busId, unitDev.devId, unitDev.funcId, registerId) {};

Register::Register(uint8_t bus, uint8_t device, uint8_t func, uint8_t registerId)
{
    uint32_t lbus  = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;
 
    /* create configuration address */
    /* See http://wiki.osdev.org/PCI (Section Configuration Space Access Mechanism #1) for details */
    configAddr = (uint32_t)((lbus << 16) | (ldevice << 11) |
              (lfunc << 8) | (registerId & 0xfc) | ((uint32_t)0x80000000));
}

uint32_t Register::get()
{
    AutoSpinLock l(&ioLock);
    outl(configAddr , PORT_PCI_CONFIG_ADDR);
    return inl(PORT_PCI_CONFIG_DATA);
}

void Register::set(uint32_t val)
{
    AutoSpinLock l(&ioLock);
    outl(configAddr , PORT_PCI_CONFIG_ADDR);
    outl(val        , PORT_PCI_CONFIG_DATA);
}

void printAllFunctions()
{
    findAllFunctions([](uint8_t bus, uint8_t device, uint8_t func)
    {
        uint32_t basicIDs = Register(bus, device, func, 0).get();
        uint32_t vendorID = basicIDs & 0xffff;
        uint32_t deviceID = basicIDs >> 16;

        printf("Vendor 0x%x Device 0x%x Func 0x%x ", vendorID, deviceID, func);

        uint32_t classInfo = Register(bus, device, func, 0x8).get();
        if((classInfo >> 24) == 1)
            printf("\t DISK!");
        else printf("\tclass = 0x%x", classInfo >> 24);

        printf(" subclass = 0x%x\n", (classInfo >> 16) & 0xff);

    });
}

void findDevOn(uint8_t bus, uint8_t device, pciFindCallback callback)
{
    uint32_t basicIDs = Register(bus, device, 0, 0).get();
    uint32_t vendorID = basicIDs & 0xffff;

    // Device does not exist
    if(vendorID == 0xFFFF) return;

    uint32_t headerType = (Register(bus, device, 0, 0xC).get() >> 16) & 0xff;
    if( (headerType & 0x80) != 0)
    {
        for(uint8_t func = 0; func < PCI_NUM_FUNC; func++)
        {
            // The specific function does not exist
            if((Register(bus, device, func, 0).get() & 0xffff) == 0xffff)
                continue;
            callback(bus, device, func);
        }
    }
    else
    {
        // Single Function
        callback(bus, device, 0);
    }
}

void findAllFunctions(pciFindCallback func)
{
    for(uint32_t i=0; i<PCI_NUM_BUS; i++)
        for(uint32_t j=0; j<PCI_NUM_DEVICE; j++)
            findDevOn(i, j, func);
}


}

