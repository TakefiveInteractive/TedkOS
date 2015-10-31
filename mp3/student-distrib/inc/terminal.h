#ifndef _TERMINAL_H
#define _TERMINAL_H

#include <stdint.h>
#include <stddef.h>

#include <inc/driver.h>

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
