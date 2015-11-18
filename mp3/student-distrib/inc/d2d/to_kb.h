#ifndef _DRIVER_TO_KEYBOARD_H
#define _DRIVER_TO_KEYBOARD_H

#include <stdint.h>
#include <stddef.h>

// Pass event to keyboard FOPS about 'newline' pressed,
//      indicating that buffer is ready to use,
//      So that keyboard FOPS resume user program from waiting in read().
//  terminal_id:  which one of the 4 terminals keyboard is writing 'enter' into.
//      But in CP2 we only have 1 terminal, so terminal_id = 0 (for CP2)
// !!!!!!!!!! DO NOT PUSH '\n' in this func !!! terminal already PUSHED one !!!!!
// !!!! Warning: this function does NOT lock term_lock !!!!

#ifdef __cplusplus
extern "C" {
#endif
void term2kb_readover(int terminal_id);
#ifdef __cplusplus
}
#endif


#endif /* _DRIVER_TO_KEYBOARD_H */
