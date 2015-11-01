#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <stdint.h>
#include <stddef.h>

#include <inc/driver.h>

#define NUM_TERMINALS           1
#define TERM_BUFFER_SIZE        128

typedef struct
{
    char displayed_char;
    
    //offset is the space this item took up on screen
    // x_offset is always positive.
    // For normal characters, x_offset = 1
    // !!! If newline occurs:
    //  x_offset = SCREEN_WIDTH - old_x
    uint32_t x_offset;

    // y_offset > 0 if and only if ONE new line occur
    // Otherwise (=0) no new line.
    uint8_t y_offset;
} term_buf_item;

// Must lock term_lock when accessing:
//  0. terminal
//  1. the term_read_buffer
//  2. isThisTerminalInUse
//  2. isThisTerminalWaitingForEnter
extern spinlock_t term_lock;

// REQUIRE: '\n' MUST be stored in this buffer, TOO.
extern ringbuf_t term_read_buf;

DEFINE_DRIVER_INIT(term);
DEFINE_DRIVER_REMOVE(term);


// These implementation will make sure cursors are moved so that kernel's output fits well with user's input

// By default the klib's printf and clear functions use this implementation.
// Change the macro redirection there to switch to old, given versions

// clear screen
void term_cls(void);

// Print one char. Must be either printable or newline character
void term_putc(uint8_t c);

#endif /* _TERMINAL_H */
