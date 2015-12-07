#ifndef _DRIVER_PCI_BUS_H
#define _DRIVER_PCI_BUS_H

#include <inc/klibs/spinlock.h>
#include <inc/klibs/function.h>

namespace pci
{

class Register
{
private:
    static spinlock_t ioLock;
    uint32_t configAddr;
public:
    Register(uint8_t bus, uint8_t device, uint8_t func, uint8_t registerId);
    uint32_t get();
    void set(uint32_t val);
};

typedef function<void (uint8_t bus, uint8_t device, uint8_t func)> pciFindCallback;

void findAllFunctions(pciFindCallback func);
void printAllFunctions();

}

#endif//_DRIVER_PCI_BUS_H
