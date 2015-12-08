
#include <inc/drivers/mouse.h>
#include <inc/drivers/common.h>
#include <inc/drivers/rtc.h>
#include <inc/drivers/kbterm.h>
#include <inc/drivers/pit.h>
#include <inc/drivers/scan_pci.h>
#include <inc/drivers/ata.h>

/* Include the header of your driver here. */

#define ADD_DRIVER(ID, NAME_STR)        \
{                                       \
    .name   = (NAME_STR),               \
    .init   = _##ID##_init,             \
    .remove = _##ID##_remove            \
}

driver_t known_drivers[] = {
    ADD_DRIVER(scanpci, "PCI driver\n"),
    ADD_DRIVER(pata, "IDE Controller driver"),
    ADD_DRIVER(kb, "Keyboard"),
    ADD_DRIVER(rtc, "Real Time Clock driver"),
    ADD_DRIVER(mouse, "Mouse"),
    ADD_DRIVER(pit, "Programmable Interrupt Timer")
};

const int num_known_drivers = sizeof(known_drivers) / sizeof(driver_t);
