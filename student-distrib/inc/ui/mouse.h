#ifndef _INC_UI_MOUSE_H_
#define _INC_UI_MOUSE_H_

#include <inc/ui/drawable.h>
#include <inc/klibs/maybe.h>

namespace ui {

class Mouse : public Drawable {
    private:
        Maybe<Container *> isDragging = Nothing;
        int dragOffsetX;
        int dragOffsetY;

    public:
        Mouse();
        void moveMouse(int dx, int dy);
        virtual bool isMovable() const { return true; }

        void leftClick();
        void leftRelease();
        void drag();
};

}

#endif
