#ifndef _DRIVER_PCI_BUS_H
#define _DRIVER_PCI_BUS_H

#include <inc/klibs/spinlock.h>
#include <inc/klibs/function.h>

namespace pci
{

// Unit device in PCI. <=> "function" <=> the smallest unit that can count as an individual device in PCI.
struct UnitDev
{
    UnitDev(uint8_t b, uint8_t d, uint8_t f)
    {
        busId = b;
        devId = d;
        funcId = f;
    }
    uint8_t busId;
    uint8_t devId;
    uint8_t funcId;
};

class Register
{
private:
    static spinlock_t ioLock;
    uint32_t configAddr;
public:
    Register(const UnitDev& unitDev, uint8_t registerId);
    Register(uint8_t bus, uint8_t device, uint8_t func, uint8_t registerId);
    uint32_t get();
    void set(uint32_t val);
};

typedef function<void (uint8_t bus, uint8_t device, uint8_t func)> pciFindCallback;

void findAllFunctions(pciFindCallback func);
void printAllFunctions();

}

#endif//_DRIVER_PCI_BUS_H
