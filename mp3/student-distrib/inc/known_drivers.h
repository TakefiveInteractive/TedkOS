
#include <inc/drivers/common.h>
#include <inc/drivers/rtc.h>
#include <inc/drivers/keyboard.h>
#include <inc/drivers/terminal.h>

/* Include the header of your driver here. */

#define ADD_DRIVER(ID, NAME_STR)		\
{										\
	.name   = (NAME_STR),				\
	.init   = _##ID##_init,				\
	.remove = _##ID##_remove			\
}

driver_t known_drivers[] = {
    ADD_DRIVER(term, "Terminal driver"),
	ADD_DRIVER(rtc, "Real Time Clock driver"),
	ADD_DRIVER(kb, "Keyboard")
	/*, ADD_DRIVER(keyboard)*/
};

const int num_known_drivers = sizeof(known_drivers) / sizeof(driver_t);
