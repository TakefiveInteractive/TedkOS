#include "ece391sysnum.h"

#define DO_CALL(name,number)   \
.GLOBL name                   ;\
name:   PUSHL	%EBX          ;\
	PUSHL	%ESI          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	MOVL	20(%ESP),%ESI ;\
	INT		$0x80         ;\
	POPL	%ESI          ;\
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
DO_CALL(ece391_fstat,SYS_FSTAT)
DO_CALL(ece391_lseek,SYS_LSEEK)
DO_CALL(ece391_fork,SYS_FORK)

/* Only Kernel threads can call this */
DO_CALL(ece391_dotask, SYS_DOTASK)

/* GUI stuff */
DO_CALL(TedCreateWindow,SYS_CREATE_WINDOW)
DO_CALL(TedSetText,SYS_SET_TEXT)
DO_CALL(TedGetText,SYS_GET_TEXT)
DO_CALL(TedElemShow,SYS_ELEM_SHOW)
DO_CALL(TedElemHide,SYS_ELEM_HIDE)
DO_CALL(TedCreateButton,SYS_CREATE_BUTTON)
DO_CALL(TedGetMessage,SYS_GET_MESSAGE)
DO_CALL(TedAttachMessageHandler,SYS_ATTACH_MSG_HANDLER)
DO_CALL(TedCreateImage,SYS_CREATE_IMAGE)
DO_CALL(TedSetImageData,SYS_SET_IMAGE_DATA)
