#ifndef _DRIVER_TO_TERMINAL_H
#define _DRIVER_TO_TERMINAL_H

#include <stdint.h>
#include <stddef.h>

// The keyboard drive should preprocess their hardware level keycode to
//   convent them into kernel level keycode and pass the keycode here.
void kb_to_term(uint32_t kernelKeycode);

// Definitions of kernel level keycodes are defined here as well.
// 1. For keys that have a PRINTABLE character, their kernel keycode is equal to ASCII.
// 2. Otherwise for keys that combine with other keys, their keycode starts
//        from 0x10000
// 3. Otherwise the keys are represented using values from 0x100 to 0xff00
// 4. The MSB represents whether the event is KEY_PRESSED or KEY_RELEASED
//        MSB 0 = PRESSED, MSB 1 = RELEASED

// define a = 'a'. define A = 'A'. define Ctrl+a = 'a' | KKC_CTRL

// KKC is short for kernel keycode


// !!! User-level keycode (anything read from fops) is different from this.
// FROM PIAZZA: We need NOT support CTRL ALT or SUPER keys, except for CTRL-L
//      And for CTRL-L the user does not need to find it in buffer.
//      THUS: user level keycode does not contain "combine-able" keys

// KKC representing whether the event was pressing or releasing a key
#define KKC_PRESS       0x00000000
#define KKC_RELEASE     0x80000000

// KKC for "combine-able" keys
#define KKC_CTRL        0x010000
#define KKC_ALT         0x020000
#define KKC_SHIFT       0x040000
#define KKC_SUPER       0x080000

// KKC for special but not "combine-able" keys
#define KKC_ENTER       0x000100
#define KKC_BACKSPACE   0x000200
#define KKC_DELETE      0x000300
#define KKC_UP          0x000400
#define KKC_DOWN        0x000500
#define KKC_LEFT        0x000600
#define KKC_RIGHT       0x000700
#define KKC_TAB         0x000800
#define KKC_CAPSLOCK    0x000900
#define KKC_ESC         0x000A00
#define KKC_F1          0x000B00
#define KKC_F2          0x000C00
#define KKC_F3          0x000D00
#define KKC_F4          0x000E00
#define KKC_F5          0x000F00
#define KKC_F6          0x001000
#define KKC_F7          0x001100
#define KKC_F8          0x001200
#define KKC_F9          0x001300
#define KKC_F10         0x001400
#define KKC_F11         0x001500
#define KKC_F12         0x001600
#define KKC_INSERT      0x001700
#define KKC_PRSCR       0x001800        //Print screen
#define KKC_PSBRK       0x001900        //Pause/Break
#define KKC_HOME        0x001A00
#define KKC_END         0x001B00
#define KKC_PAGEUP      0x001C00
#define KKC_PAGEDOWN    0x001D00
//#define KKC_            0x001E00
//#define KKC_            0x001F00
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000
//#define KKC_            0x002000


// and so on

#endif /* _DRIVER_TO_TERMINAL_H */
