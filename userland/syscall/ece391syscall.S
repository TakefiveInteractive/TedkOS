#include "../../shared/ece391sysnum.h"

#include "../../shared/ece391syscall.S"

/* Call the main() function, then halt with its return value. */

.GLOBAL _start
_start:
	CALL	main
	PUSHL	%EAX
	CALL	exit

