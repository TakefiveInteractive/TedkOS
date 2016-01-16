#include <stdint.h>
#include <stddef.h>
#include <inc/drivers/kbterm.h>
#include <inc/drivers/kkc.h>
#include <inc/drivers/common.h>
#include <inc/klibs/lib.h>
#include <inc/klibs/spinlock.h>
#include <inc/proc/tasks.h>
#include <inc/proc/sched.h>
#include <inc/ui/compositor.h>
#include <inc/init.h>

using ui::Compositor;

//********** DEFINE HANDLERS for special keycodes **********
extern char ascii_shift_table[128];
// -------- C++ classes ------------
namespace Term
{
    Term::Term()
    {
        // Empty constructor?
    }

    void Term::keyDown(uint32_t kkc, bool capslock)
    {

    }

    void Term::keyUp(uint32_t kkc, bool capslock)
    {

    }

    uint8_t* Term::enableVidmap(const struct _thread_pcb* theThread)
    {
        return getTermPainter()->enableVidmap(theThread);
    }

    void Term::tryDisableVidmap(const struct _thread_pcb* theThread)
    {
        getTermPainter()->tryDisableVidmap(theThread);
    }

    void Term::tryMapVidmap(const thread_pcb* pcbToLoadMemmap)
    {
        getTermPainter()->tryMapVidmap(pcbToLoadMemmap);
    }

    // Warning: these handlers DO NOT LOCK spinlocks !!!
    void Term::nolock_enter_handler(uint32_t keycode)
    {
        term_buf_item newLine;
        newLine.displayed_char = '\n';
        if (next_char_y < SCREEN_HEIGHT - 1)
        {
            next_char_y++;
            next_char_x = 0;
        }
        else
        {
            next_char_x = 0;
            getTermPainter()->scrollDown();
        }
        //  kb_read must add '\n' by itself.
        term_buf[term_buf_pos++] = newLine;

        if (UserWaitingRead)
        {
            // This must only occur at process 0
            UserWaitingRead = false;
            auto& proc = ProcessDesc::get(OwnedByPid);
            cpu0_memmap.loadProcessMap(&proc);
            getRegs(proc.mainThreadInfo)->eax = helpFinishRead(UserWaitingBuffer, UserWaitingLen);
            cpu0_memmap.loadProcessMap(getCurrentThreadInfo()->getProcessDesc());
            scheduler::unblock(proc.mainThreadInfo);
        }
        else term_buf_pos = 0;
    }

    void Term::nolock_backspace_handler(uint32_t keycode)
    {
        if(term_buf_pos > 0)
        {
            term_buf_item* i = &term_buf[term_buf_pos - 1];
            if(i->y_offset)
            {
                getTermPainter()->clearLine(next_char_y);
                next_char_y--;
                next_char_x = SCREEN_WIDTH - i->x_offset;

                // Must only use this direction (j-- cause sign issues)
                for(uint32_t j = next_char_x; j <= SCREEN_WIDTH - 1; j++)
                    getTermPainter()->showChar(j, next_char_y, ' ');
            }
            else
            {
                uint32_t orig_x = next_char_x;
                next_char_x = orig_x - i->x_offset;

                // Must only use this direction (j-- cause sign issues)
                for(uint32_t j = next_char_x; j <= orig_x; j++)
                    getTermPainter()->showChar(j, next_char_y, ' ');
            }
            term_buf_pos--;
        }
    }

    void Term::nolock_tab_handler(uint32_t keycode)
    {
        if(term_buf_pos >= TERM_BUFFER_SIZE - 1)
            return;
        uint32_t old_x;
        term_buf_item rec;
        rec.displayed_char = '\t';
        old_x = next_char_x;
        next_char_x  += 4 - (next_char_x % TAB_WIDTH);
        if(next_char_x >= SCREEN_WIDTH)
        {
            rec.x_offset = SCREEN_WIDTH - old_x;
            rec.y_offset = 1;

            // Linux VM is bad at this.
            // I decided to make any tab start at newline.
            next_char_x = 0;
            if(next_char_y < SCREEN_HEIGHT - 1)
                next_char_y++;
            else getTermPainter()->scrollDown();
        }
        else
        {
            rec.x_offset = next_char_x - old_x;
            rec.y_offset = 0;
        }
        term_buf[term_buf_pos++] = rec;
    }

    void Term::nolock_putc(uint8_t c)
    {
        // Re-Initialize the buffer. (Things printed from screens are not deletable)
        term_buf_pos = 0;

        if(c == '\0') {
            // We should not print spaces for any trailing null characters
            ;
        } else if(c == '\n' || c == '\r') {
            if(next_char_y < SCREEN_HEIGHT - 1)
            {
                next_char_y++;
                next_char_x = 0;
            }
            else
            {
                next_char_x = 0;
                getTermPainter()->scrollDown();
            }
        } else if (c == '\t') {
            next_char_x += 4 - (next_char_x % TAB_WIDTH);
            if(next_char_x >= SCREEN_WIDTH)
            {
                next_char_x = next_char_x % SCREEN_WIDTH;
                if(next_char_y < SCREEN_HEIGHT - 1)
                    next_char_y++;
                else getTermPainter()->scrollDown();
            }
        } else {
            getTermPainter()->showChar(next_char_x, next_char_y, c);
            next_char_x++;
            if(next_char_x == SCREEN_WIDTH)
            {
                next_char_x = 0;
                if(next_char_y < SCREEN_HEIGHT - 1)
                    next_char_y++;
                else getTermPainter()->scrollDown();
            }
        }
        getTermPainter()->setCursor(next_char_x, next_char_y);
    }

    int32_t Term::read(void* fdData, uint8_t *buf, int32_t nbytes)
    {
        AutoSpinLock lock(&term_lock);

        char* cbuf = (char*) buf;

        // Firstly: check owner
        if (OwnedByPid == -1)
        {
            OwnedByPid = getCurrentThreadInfo()->getProcessDesc()->getPid();
            UserWaitingRead = false;
        }
        else if (OwnedByPid != getCurrentThreadInfo()->getProcessDesc()->getPid())
        {
            return -EFOPS;
        }

        // If currently buffer is not enough, wait.
        if (term_buf_pos < (size_t) nbytes)
        {
            UserWaitingRead = true;
            UserWaitingBuffer = cbuf;
            UserWaitingLen = nbytes;

            // Switch to IDLE thread
            scheduler::block(getCurrentThreadInfo());

            // This return value will be clobbered later upon keyboard interrupt.
            return 0;
        }
        else return helpFinishRead(cbuf, nbytes);
    }

    int32_t Term::helpFinishRead(char* buffer, int32_t nbytes)
    {
        if(!buffer)
            return -EFOPS;
        volatile int32_t copylen;
        if((size_t) nbytes < term_buf_pos)
            copylen = nbytes;
        else copylen = term_buf_pos;

        // Now that we do not need term_buf_pos anymore, we clear its value here.
        term_buf_pos = 0;

        volatile int32_t i = 0;
        for(; i < copylen; i++)
        {
            buffer[i] = term_buf[i].displayed_char;
        }
        return i;
    }

    int32_t Term::write(void* fdData, const uint8_t *buf, int32_t nbytes)
    {
        AutoSpinLock l(&term_lock);
        int32_t i;
        const char* cbuf = (const char*) buf;
        for(i = 0; i < nbytes; i++)
        {
            if(cbuf[i] == 0)    // end of string
                return i;
            nolock_putc(cbuf[i]);
        }
        return nbytes;
    }

    void Term::nolock_cls()
    {
        term_buf_pos = 0;
        next_char_x = next_char_y = 0;
        getTermPainter()->clearScreen();
        getTermPainter()->setCursor(next_char_x, next_char_y);
    }

    // ============= PUBLIC ===============

    void Term::cls()
    {
        AutoSpinLock l(&term_lock);
        nolock_cls();
    }

    void Term::putc(uint8_t c)
    {
        AutoSpinLock l(&term_lock);
        nolock_putc(c);
    }

    void Term::setOwner(bool lock, int32_t upid)
    {
        uint32_t flags;
        if(lock)
            spin_lock_irqsave(&term_lock, flags);
        OwnedByPid = upid;
        UserWaitingRead = false;
        UserWaitingBuffer = NULL;
        UserWaitingLen = -1;
        if(lock)
            spin_unlock_irqrestore(&term_lock, flags);
    }

    void Term::canBeOwnedBy(int32_t tid, function<void (bool result)> callback)
    {
        AutoSpinLock l(&term_lock);
        callback(OwnedByPid == -1 || OwnedByPid == tid);
    }

    void Term::key(uint32_t kkc, bool capslock)
    {
        AutoSpinLock l(&term_lock);
        if(kkc & ~KKC_ASCII_MASK & ~KKC_SHIFT)
        {
            // Handle possible shortcuts here.
            if(kkc & (~KKC_SPECIAL_MASK))
            {
                if(kkc == (KKC_CTRL | (uint32_t)'l'))
                {
                    next_char_x = next_char_y = 0;
                    term_buf_pos = 0;
                    getTermPainter()->clearScreen();
                }
            }
            else switch(kkc & KKC_SPECIAL_MASK)
            {
            case KKC_BACKSPACE:
                nolock_backspace_handler(kkc);
                break;
            case KKC_TAB:
                nolock_tab_handler(kkc);
                break;
            case KKC_ENTER:
                nolock_enter_handler(kkc);
                break;
            default:
                break;
            }
        }
        else
        {
            // Handle normal characters here.
            // < SIZE -1 because '\n' takes space
            if(term_buf_pos < TERM_BUFFER_SIZE - 1)
            {
                // In this case we have directly printable characters
                char c = (char)(kkc & KKC_ASCII_MASK);
                uint32_t old_x;
                term_buf_item rec;
                if(kkc & KKC_SHIFT)
                {
                    uint32_t ascii_part = c;
                    // If this ascii has a shift, then do shift.
                    if(ascii_shift_table[ascii_part])
                        c = ascii_shift_table[ascii_part];
                }
                if(capslock)
                {
                    if('a' <= c && c <= 'z')
                        c = c - 'a' + 'A';
                    else if('A' <=c && c <= 'Z')
                        c = c - 'A' + 'a';
                }

                getTermPainter()->showChar(next_char_x, next_char_y, c);

                rec.displayed_char = c;
                old_x = next_char_x;
                next_char_x++;
                if(next_char_x == SCREEN_WIDTH)
                {
                    rec.x_offset = SCREEN_WIDTH - old_x;
                    rec.y_offset = 1;
                    next_char_x = 0;
                    if(next_char_y < SCREEN_HEIGHT - 1)
                        next_char_y++;
                    else getTermPainter()->scrollDown();
                }
                else
                {
                    rec.x_offset = next_char_x - old_x;
                    rec.y_offset = 0;
                }
                term_buf[term_buf_pos++] = rec;
            }
        }
        getTermPainter()->setCursor(next_char_x, next_char_y);
    }

    // +++++++++ TextTerm ++++++++++

    // Private function: no need to lock.
    TermPainter* TextTerm::getTermPainter()
    {
        return &painter;
    }

    // PUBLIC function: Lock the spinlock !
    void TextTerm::show()
    {
        AutoSpinLock l(&term_lock);
        painter.show();
    }

    void TextTerm::hide()
    {
        AutoSpinLock l(&term_lock);
        painter.hide();
    }

    TextTerm::TextTerm() : Term()
    {
        // Empty constructor?
    }
}
