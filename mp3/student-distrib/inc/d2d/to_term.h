#ifndef _DRIVER_TO_TERMINAL_H
#define _DRIVER_TO_TERMINAL_H

// The keyboard drive should preprocess their hardware level keycode to
//   convent them into kernel level keycode and pass the keycode here.
void kb_to_term(int32_t kenerlKeycode);

// Definitions of kernel level keycodes are defined here as well.
// 1. For keys that have a PRINTABLE character, their kernel keycode is equal to ASCII.
// 2. Otherwise for keys that combine with other keys, their keycode starts
//        from 0x10000
// 3. Otherwise the keys are represented using values from 0x100 to 0xff00

// define a = 'a'. define A = 'a' | KKC_SHIFT

// KKC is short for kernel keycode

#define KKC_CTRL    0x10000
#define KKC_ALT     0x20000
#define KKC_SHIFT   0x40000

// and so on

#endif /* _DRIVER_TO_TERMINAL_H */
