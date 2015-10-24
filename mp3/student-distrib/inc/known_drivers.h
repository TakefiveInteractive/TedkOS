
#include <inc/driver.h>
#include <inc/rtc.h>

/* Include the header of your driver here. */

#define ADD_DRIVER(ID, NAME_STR)		\
{										\
	.name   = (NAME_STR),				\
	.init   = _##ID##_init,				\
	.remove = _##ID##_remove			\
}

driver_t known_drivers[] = {
	ADD_DRIVER(rtc, "Real Time Clock driver"),
	ADD_DRIVER(kb,"Keyboard")
	/*, ADD_DRIVER(keyboard)*/
};
