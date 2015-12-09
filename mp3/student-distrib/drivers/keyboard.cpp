#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/kbterm.h>
#include <inc/drivers/kkc.h>
#include <inc/d2d/k2m.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/fs/dev_wrapper.h>
#include <inc/proc/sched.h>
#include <inc/proc/tasks.h>
#include <inc/init.h>

using filesystem::register_devfs;

#define KB_PORT 0x60
#define KB_IRQ_NUM 1
#define KB_ID 1
#define KD_POLICY 0

//------------------------- DEFINE PRIVATE : BASIC KB handler, and HIGHEST SHORTCUT PARSER -----------------------
// !!! All these private "static" functions will NOT lock the keyb_lock !!!

namespace KeyB
{
    spinlock_t keyboard_lock = SPINLOCK_UNLOCKED;
    KbClients clients;
}
using KeyB::clients;
using KeyB::keyboard_lock;

static size_t currClient = 0;

static bool caps_locked = false;

// This field is used to tell whether COMBINATION key is pressed
//      If so, then it also contains all COMBINATION keys pressed.
//      Note that pending_kc's MSB is never 1 (released)
static uint32_t pending_kc = 0;

// Directly called by interrupt handler.

// This filter is executed before any other IEvent.
//  it will:
//  1. prevent typing by not sending events to terminal,
//          after too many chars are typed in.
//  2. handler shortcuts
//  3. Otherwise, pass event to all terminals
//  4. It always handles the kb_read buffer.
static void handle(uint32_t kernelKeycode);

// Helper for "handle". true = this key should NOT be passed to IEvent::key()
// Alt + # is handled here, but Ctrl+ l is handled in terminal.
static bool handleShortcut(uint32_t kernelKeycode);

static void switchToTextTerm(size_t clientId);

// -------------- An important helper      ------------------

namespace KeyB
{
    Term::TextTerm* getFirstTextTerm()
    {
        return &(clients.textTerms[0]);
    }
}
using KeyB::getFirstTextTerm;

// ======== over =========


/*
 * See http://wiki.osdev.org/PS/2_Keyboard
 */
uint32_t KBascii[128] =
{
    /* keycode here is all pressed keycode */
    0,/* unused,nothing happened */
    KKC_ESC,/* esc */
    '1','2','3','4','5','6','7','8','9','0','-','=',
    KKC_BACKSPACE,/* Backspace */
    KKC_TAB,/* Tab */
    'q','w','e','r','t','y','u','i','o','p','[',']',
    KKC_ENTER,	/* Enter key */
    KKC_CTRL,	/* left Control */
    'a','s','d','f','g','h','j','k','l',
    ';',	/* 39 */
    '\'', '`',
    KKC_SHIFT,	/* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', ',', '.', '/',
    KKC_SHIFT,	/* Right shift */
    '*',
    KKC_ALT,	/* left Alt */
    ' ',/* Space */
    KKC_CAPSLOCK,	/* Caps lock */
    KKC_F1, KKC_F2, KKC_F3, KKC_F4, KKC_F5, KKC_F6, KKC_F7, KKC_F8, KKC_F9, KKC_F10,
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    KKC_HOME,	/* Home key */
    KKC_UP,	/* Up Arrow */
    KKC_PAGEUP,	/* Page Up */
    '-',
    KKC_LEFT,	/* Left Arrow */
    0,
    KKC_RIGHT,	/* Right Arrow */
    '+',
    KKC_END,	/* 79 - End key*/
    KKC_DOWN,	/* Down Arrow */
    KKC_PAGEDOWN,	/* Page Down */
    KKC_INSERT,	/* Insert Key */
    KKC_DELETE,	/* Delete Key */
    0, 0, 0,
    KKC_F11,	/* F11 Key */
    KKC_F12,	/* F12 Key */
    0,	/* All other keys are undefined */
};

// Special key Prefix is 0xE0
#define SPECIAL_PREFIX      0xE0
#define RELEASE_OFFSET      0x80

int8_t  pending_special = 0;
int kb_handler(int irq, unsigned int saved_reg);

DEFINE_DRIVER_INIT(kb) {
    AutoSpinLock l(&KeyB::keyboard_lock);

    // bind handler to pic
    bind_irq(KB_IRQ_NUM,KB_ID,kb_handler,KD_POLICY);

    // register FOPS
    register_devfs("term", []() { return Term::FOps::getNewInstance(); });
    register_devfs("keyb", []() { return KeyB::FOps::getNewInstance(); });

    getFirstTextTerm()->show();

    return;
}

DEFINE_DRIVER_REMOVE(kb) {
    AutoSpinLock l(&KeyB::keyboard_lock);

    // rm handler from pic
    unbind_irq(KB_IRQ_NUM, KB_ID);
    pending_special = 0;

    // reset terminals
    for(size_t i = 0; i < KeyB::KbClients::numClients; i++)
        clients.textTerms[i].cls();

    return;
}

/* keyboard_handler
 * description:
 *      Handle keyboard interrupts
 * input:
 *      int irq, pt_reg* saved_reg(not used)
 * output,return:
 *      currently the value is ignored, 0 for success.
 *      Look at <inc/error.h> for more error codes.
 * side effect:
 *      changes keyboard buffer (if we have a buffer)
 *      print keyboard character to terminal
 */
int kb_handler(int irq, unsigned int saved_reg)
{
    uint8_t keyboard_scancode;
    AutoSpinLock(&KeyB::keyboard_lock);

    keyboard_scancode = inb(KB_PORT);                           //read the input

    //!!! WARNING: We should not use "keyboard_scancode & 0x80" anymore
    //!!!   Because that filters out the events for release keys
    //!!!   BUT we need to know when keys like SHIFT, ALT are released

    if(keyboard_scancode == 0) {
        return 0;
    }
    if(keyboard_scancode == SPECIAL_PREFIX) {
        pending_special = 1;
        return 0;
    }

    // On some keyboards, keypad enter's 2nd code is not 0x1C
    //   They use 0x18 as 2nd code, which conflicts with 'o'
    if(pending_special && keyboard_scancode == 0x18) {
        pending_special = 0;
        return 0;
    }
    pending_special = 0;
    if ((keyboard_scancode & RELEASE_OFFSET) == 0 ) {                     // pressed
        uint32_t kernel_keycode = KBascii[keyboard_scancode];
        handle(kernel_keycode|KKC_PRESS);
    }

    if (keyboard_scancode & RELEASE_OFFSET) {                             // released
        uint32_t kernel_keycode = KBascii[keyboard_scancode & (~RELEASE_OFFSET)];
        handle(kernel_keycode|KKC_RELEASE);
    }

    return 0;
}

//------------------------- BASIC KB handler, and HIGHEST SHORTCUT PARSER -----------------------
// !!! All these private "static" functions will NOT lock the keyb_lock !!!

// This filter is executed before any other IEvent.
//  it will:
//  1. prevent typing by not sending events to terminal,
//          after too many chars are typed in.
//  2. handler shortcuts
//  3. Otherwise, pass event to all terminals
//  4. It always handles the kb_read buffer.

// The entry point into C++ world for all keyboard events.
/* The param is PARTIAL kernelKeycode, because keyboard does not combine keys.
 *  'a' is ok. KKC_ENTER is ok,
 *  but no one will pass in SHIFT|'a'.
 */
void handle(uint32_t kernelKeycode)
{
    uint32_t ascii_part, special_part, combine_part;

    // *_part are booleans to represent the type of key being pressed, or released
    // The rules to extract the type are explained in inc/driver/kkc.h
    ascii_part   = kernelKeycode & KKC_ASCII_MASK;
    special_part = kernelKeycode & KKC_SPECIAL_MASK;
    combine_part = kernelKeycode & KKC_COMBINE_MASK;

    // If this event represents releasing a key
    if ((kernelKeycode & KKC_RELEASE) != 0)
    {
        // calculate change so that (pending_kc & change) is the NEW pending_kc
        // If neither part exists, nothing should change, thus default to ~0x0
        uint32_t change = 0xFFFFFFFF;

        /* WE ASSUME keyboard supports auto-repeat interrupts. */

        change &= ~combine_part;

        pending_kc &= change;

        clients[currClient]->keyUp(kernelKeycode, caps_locked);
    }
    else
    {
        // A rule: everytime a new ascii/special key is pressed,
        //     old pressed ascii/special key is discarded
        // Thus we only put combine_part in pending_kc
        if (combine_part)
            pending_kc = pending_kc | combine_part;
        else if (ascii_part || special_part)             // avoid 0x0
        {
            clients[currClient]->keyDown(pending_kc | special_part | ascii_part, caps_locked);
            if (!handleShortcut(pending_kc | special_part | ascii_part))
                clients[currClient]->key(pending_kc | special_part | ascii_part, caps_locked);
        }
    }
}

bool handleShortcut(uint32_t kernelKeycode)
{
    if((kernelKeycode & KKC_SPECIAL_MASK) == KKC_CAPSLOCK)
    {
        caps_locked = !caps_locked;
        return true;
    }
    else switch(kernelKeycode)
    {
    case KKC_ALT | KKC_F1:
        switchToTextTerm(0);
        return true;
    case KKC_ALT | KKC_F2:
        switchToTextTerm(1);
        return true;
    case KKC_ALT | KKC_F3:
        switchToTextTerm(2);
        return true;
    case KKC_ALT | KKC_F4:
        switchToTextTerm(3);
        return true;
    case KKC_ALT | KKC_F5:
        switchToTextTerm(4);
        return true;
    case KKC_ALT | KKC_F6:
        switchToTextTerm(5);
        return true;

    default:
        // Currently we do NOT handle other cases of COMBINE+SPECIAL
        return false;
    }
}

void switchToTextTerm(size_t clientId)
{
    currClient = clientId;
    ((Term::TextTerm*)(clients[clientId]))->show();
}

// -------- PUBLIC C functions used by printf --------

// clear screen
void term_cls(void)
{
    if(!canUseCpp)
    {
        orig_clear();
    }
    else if(!isFallbackTerm && getCurrentThreadInfo()->getProcessDesc()->currTerm != NULL)
    {
        getCurrentThreadInfo()->getProcessDesc()->currTerm->cls();
    }
    else
    {
        getFirstTextTerm()->cls();
    }
}

// Print one char. Must be either printable or newline character
void term_putc(uint8_t c)
{
    if(!canUseCpp)
    {
        orig_putc(c);
    }
    else if(!isFallbackTerm && getCurrentThreadInfo()->getProcessDesc()->currTerm != NULL)
    {
        getCurrentThreadInfo()->getProcessDesc()->currTerm->putc(c);
    }
    else
    {
        getFirstTextTerm()->putc(c);
    }
}

// ---------
namespace KeyB
{
    KbClients::KbClients()
    {
        for(size_t i=0; i<numClients; i++)
            clients[i] = &textTerms[i];
    }
    KbClients::~KbClients() {}
    IEvent* KbClients::operator [] (size_t i)
    {
        return clients[i];
    }
}
