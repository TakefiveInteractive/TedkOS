
#include <inc/driver.h>
#include <inc/rtc.h>
#include <inc/keyboard.h>
#include <inc/terminal.h>
#include <inc/mouse.h>

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
	ADD_DRIVER(kb, "Keyboard"),
	ADD_DRIVER(mouse,"Mouse")
	/*, ADD_DRIVER(keyboard)*/
};

const int num_known_drivers = sizeof(known_drivers) / sizeof(driver_t);
