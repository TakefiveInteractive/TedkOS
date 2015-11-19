#include "../../shared/ece391sysnum.h"

#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET

/* the system call library wrappers */
DO_CALL(ece391_halt,SYS_HALT)
DO_CALL(ece391_execute,SYS_EXECUTE)
DO_CALL(ece391_read,SYS_READ)
DO_CALL(ece391_write,SYS_WRITE)
DO_CALL(ece391_open,SYS_OPEN)
DO_CALL(ece391_close,SYS_CLOSE)
DO_CALL(ece391_getargs,SYS_GETARGS)
DO_CALL(ece391_vidmap,SYS_VIDMAP)
DO_CALL(ece391_set_handler,SYS_SET_HANDLER)
DO_CALL(ece391_sigreturn,SYS_SIGRETURN)
DO_CALL(ece391_sbrk,SYS_SBRK)


/* Call the main() function, then halt with its return value. */

.GLOBAL _start
_start:
	CALL	main
	PUSHL	%EAX
	CALL	exit

