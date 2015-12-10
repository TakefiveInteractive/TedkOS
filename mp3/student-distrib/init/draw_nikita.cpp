#include "draw_nikita.h"
#include <inc/ui/compositor.h>

using ui::Compositor;

KeyB::IEvent* draw_nikita()
{
    Compositor *comp = Compositor::getInstance();
    comp->drawNikita();
    return comp;
}

