#ifndef _KB_TERM_DRIVER_H
#define _KB_TERM_DRIVER_H

#include <stdint.h>
#include <stddef.h>

#include <inc/error.h>
#include <inc/drivers/common.h>

#ifdef __cplusplus

#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/spinlock.h>
#include <inc/klibs/maybe.h>
#include <inc/klibs/lib.h>
#include <inc/i8259.h>
#include <inc/syscalls/exec.h>
#include <inc/syscalls/halt.h>

#include <inc/fs/filesystem.h>
#include <inc/fs/fops.h>


namespace KeyB
{
    // By default, all functions do nothing.
    class IEvent
    {
    public:
        virtual void key(uint32_t kkc, bool capslock) = 0;

        // Down and Up cuts changes to ONE single key at a time.
        virtual void keyDown(uint32_t kkc, bool capslock) = 0;
        virtual void keyUp(uint32_t kkc, bool capslock) = 0;
    };
    class FOps : public IFOps
    {
    private:
        FOps();

    public:
        virtual int32_t read(FsSpecificData *fdData, uint8_t *buf, int32_t bytes);
        virtual int32_t write(FsSpecificData *fdData, const uint8_t *buf, int32_t bytes);

        /* close */
        virtual ~FOps();
        static Maybe<IFOps *> getNewInstance();
    };
}

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


/*
 *
 * About locks between KeyB, Term, and Painter:
 *
 *  Every method encapsulated as public will lock a spinlock.
 *
 *  As long as the direction of all function calls is the same, no problem.
 *
 *  Usually KeyB calls Term, Term calls Painter. (Thus Painter should never
 *  call Term or KeyB, and Term should never call KeyB)
 *
 *  TODO:
 *
 *  About syscall:
 *     read and write calls Term, so during that we MUST CLI (prevent KeyB int)
 *          SOLUTION: syscall to term and keyb must cli()
 *  About exception:
 *      if any exception occurs inside Term, or Painter, dead lock happens
 *          IF AND ONLY IF exception handler use printf implemented by Term
 *      SOLUTION:
 *          Because Term calls only happen inside Interrupts or during booting (before init),
 *          exception handler should detect such cases, where it then
 *          uses a fallback method to clear screen and print information at line 0.
 *          (Because this is kernel crash, we should halt the whole machine anyway...)
 *
 */

#define TAB_WIDTH       4
#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25
#define TEXT_STYLE      0x7
#define TXT_VMEM_OFF    0xB8000

namespace Term
{
    class TermPainter
    {
    public:

        // Returns a VIRT pointer to USER-RW page
        virtual uint8_t* enableVidmap() = 0;
        virtual void disableVidmap() = 0;
        virtual bool isVidmapEnabled();

        // !! ALL Painters have their own lock, separate from Term's lock !!
        virtual void clearScreen() = 0;
        virtual void scrollDown() = 0;
        virtual void setCursor(uint32_t x, uint32_t y) = 0;
        virtual void showChar(uint32_t x, uint32_t y, uint8_t c) = 0;
        virtual void clearLine(uint32_t y) = 0;
    };
    class TextModePainter : public TermPainter
    {
        static TextModePainter* currShowing;

        // We do NOT need a lock for main Vmem, because no other functions operate on text vmem,
        //          and because using lock in each instance is sufficient.
        //static spinlock_t txtVMemLock = SPINLOCK_UNLOCKED;

        spinlock_t lock = SPINLOCK_UNLOCKED;
        uint8_t backupBuffer[SCREEN_WIDTH * SCREEN_HEIGHT * 2];
        uint32_t cursorX = 0, cursorY = 0;
        bool isLoadedInVmem;
        bool bIsVidmapEnabled = false;

        // this is just helper. this does NOT lock spinlock
        uint8_t* videoMem();

        // this helper simply sets the cursor, without considering lock or ownership AT ALL.
        void helpSetCursor(uint32_t x, uint32_t y);
    public:
        TextModePainter();
        virtual void show();

        // Returns a VIRT pointer to USER-RW page
        virtual uint8_t* enableVidmap();
        virtual void disableVidmap();
        virtual bool isVidmapEnabled();

        virtual void clearScreen();
        virtual void scrollDown();
        virtual void setCursor(uint32_t x, uint32_t y);
        virtual void showChar(uint32_t x, uint32_t y, uint8_t c);
        virtual void clearLine(uint32_t y);

    };

    class FOps : public IFOps
    {
    private:
        FOps();

    public:
        virtual int32_t read(FsSpecificData *fdData, uint8_t *buf, int32_t bytes);
        virtual int32_t write(FsSpecificData *fdData, const uint8_t *buf, int32_t bytes);

        /* close */
        virtual ~FOps();
        static Maybe<IFOps *> getNewInstance();
    };

    class Term : public KeyB::IEvent
    {
        friend int32_t syscall::exec::do_exec(const char* file);
        friend int32_t syscall::halt::syshalt(uint32_t retval);
    protected:
        static const size_t TERM_BUFFER_SIZE = 128;

        // The coordinate to display the next char at.
        uint32_t next_char_x = 0;
        uint32_t next_char_y = 0;

        // The lock must be called when operating on the screen.
        // Must lock term_lock when accessing:
        //  0. terminal
        //  1. the term_read_buffer
        //  2. isThisTerminalInUse
        //  2. isThisTerminalWaitingForEnter
        spinlock_t term_lock = SPINLOCK_UNLOCKED;

        // this is actually the position of NEXT NEW char.
        volatile size_t term_buf_pos = 0;
        term_buf_item term_buf[TERM_BUFFER_SIZE] = {};

        virtual TermPainter* getTermPainter() = 0;

        virtual void nolock_tab_handler(uint32_t keycode) final;
        virtual void nolock_enter_handler(uint32_t keycode) final;
        virtual void nolock_backspace_handler(uint32_t keycode) final;

        int32_t OwnedByPid = -1;
        bool UserWaitingRead = false;
        char* UserWaitingBuffer = NULL;
        int32_t UserWaitingLen = -1;

        virtual void nolock_putc(uint8_t c) final;
        virtual void nolock_cls() final;
        virtual int32_t helpFinishRead(char* buffer, int32_t nbytes) final;
    public:
        Term();

        // Returns a VIRT pointer to USER-RW page
        virtual uint8_t* enableVidmap() final;
        virtual void disableVidmap() final;
        virtual bool isVidmapEnabled() final;

        virtual void key(uint32_t kkc, bool capslock) final;
        virtual void keyDown(uint32_t kkc, bool capslock) final;
        virtual void keyUp(uint32_t kkc, bool capslock) final;

        // These are used by printf in klibs
        // calling these operations WILL clear the buffer.
        virtual void putc(uint8_t c) final;
        virtual void cls() final;

        // The following will be called by Term::FOps and KeyB::FOps
        virtual int32_t read(void* fdData, uint8_t *buf, int32_t bytes) final;
        virtual int32_t write(void* fdData, const uint8_t *buf, int32_t bytes) final;

        // Used by keyboard close fops setOwner will not block.
        //   if upid = -1, then keyboard is owned by no body.
        virtual void setOwner(int32_t upid) final;
    };

    class TextTerm : public Term
    {
    protected:
        TextModePainter painter;

        virtual TermPainter* getTermPainter();
    public:
        TextTerm();

        // this will switch text mode window to this term
        // this function does not help switch from GUI to text mode
        void show();
    };
}

namespace KeyB
{
    class KbClients
    {
    public:
        static const size_t numClients = 4;
    private:
        // In order to support GUI later, here we do NOT directly use TermImpl as type of clients
        IEvent* clients[numClients];
    public:
        Term::TextTerm textTerms[4] = {};

        KbClients();
        virtual ~KbClients();

        IEvent* operator [] (size_t i);
    };

    extern Term::TextTerm* getFirstTextTerm();
    extern spinlock_t keyboard_lock;
    extern KbClients clients;
}

extern "C" {

#endif //ifdef __cplusplus

// These implementation will make sure cursors are moved so that kernel's output fits well with user's input

// By default the klib's printf and clear functions use this implementation.
// Change the macro redirection there to switch to old, given versions

// clear screen
extern void term_cls(void);

// Print one char. Must be either printable or newline character
extern void term_putc(uint8_t c);

#ifdef __cplusplus
}
#endif //ifdef __cplusplus

DEFINE_DRIVER_INIT(kb);
DEFINE_DRIVER_REMOVE(kb);


#endif//_KB_TERM_DRIVER_H

