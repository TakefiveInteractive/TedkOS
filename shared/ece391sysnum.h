#if !defined(ECE391SYSNUM_H)
#define ECE391SYSNUM_H

#define SYS_HALT        1
#define SYS_EXECUTE     2
#define SYS_READ        3
#define SYS_WRITE       4
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_GETARGS     7
#define SYS_VIDMAP      8
#define SYS_SET_HANDLER 9
#define SYS_SIGRETURN   10
#define SYS_SBRK        11
#define SYS_FSTAT       12
#define SYS_LSEEK       13

// Switch to an existing pcb's thread
#define SYS_DOTASK 		14

#define SYS_FORK        15

// GUI functions
#define SYS_CREATE_WINDOW			0x100
#define SYS_SET_TEXT				0x101
#define SYS_GET_TEXT				0x102
#define SYS_ELEM_SHOW				0x103
#define SYS_ELEM_HIDE				0x104

#define SYS_CREATE_BUTTON			0x105

#define SYS_GET_MESSAGE				0x106
#define SYS_ATTACH_MSG_HANDLER		0x107

#define SYS_CREATE_IMAGE			0x108
#define SYS_SET_IMAGE_DATA			0x109

#endif /* ECE391SYSNUM_H */
