#include "draw_nikita.h"
#include <stdint.h>
#include <stddef.h>
#include <inc/ui/compositor.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/x86/err_handler.h>
#include <inc/ece391syscall.h>

using ui::Compositor;

class SimpleIEvent : public KeyB::IEvent
{
public:
    virtual void key(uint32_t kkc, bool capslock)
    {
    }

    // Down and Up cuts changes to ONE single key at a time.
    virtual void keyDown(uint32_t kkc, bool capslock)
    {
    }

    virtual void keyUp(uint32_t kkc, bool capslock)
    {
    }

    virtual void show()
    {
        Compositor *comp = Compositor::getInstance();
        comp->enterVideoMode();

        // TODO: FIXME: do not draw everytime (directly copy from build buffer)
        comp->drawNikita();
    }
};


KeyB::IEvent* draw_nikita()
{
    return new SimpleIEvent();
}
