#if !defined(ECE391SYSCALL_H)
#define ECE391SYSCALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* All calls return >= 0 on success or -1 on failure. */

/*
 * Note that the system call for halt will have to make sure that only
 * the low byte of EBX (the status argument) is returned to the calling
 * task.  Negative returns from execute indicate that the desired program
 * could not be found.
 */
extern int32_t ece391_halt (uint8_t status);
extern int32_t ece391_execute (const uint8_t* command);
extern int32_t ece391_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t ece391_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t ece391_open (const uint8_t* filename);
extern int32_t ece391_close (int32_t fd);
extern int32_t ece391_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t ece391_vidmap (uint8_t** screen_start);
extern int32_t ece391_set_handler (int32_t signum, void* handler);
extern int32_t ece391_sigreturn (void);
extern int32_t ece391_dotask (int32_t tid);
extern int32_t ece391_fork (void);

// GUI functions
extern void * TedCreateWindow(int32_t width, int32_t height);
extern int32_t TedSetText(void *elem, char *text);
extern int32_t TedGetText(void *elem, char *buffer);
extern int32_t TedElemShow(void *elem);
extern int32_t TedElemHide(void *elem);

extern void * TedCreateButton(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y);

extern int32_t TedGetMessage(void *message);
extern int32_t TedAttachMessageHandler(void *elem, void *fn);

extern void * TedCreateImage(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y);
extern int32_t TedSetImageData(void *elem, void *data);

#ifdef __cplusplus
}
#endif

enum signums {
    DIV_ZERO = 0,
    SEGFAULT,
    INTERRUPT,
    ALARM,
    USER1,
    NUM_SIGNALS
};

#endif /* ECE391SYSCALL_H */


