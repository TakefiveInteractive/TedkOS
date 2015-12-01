#ifndef _INC_UI_MOUSE_H_
#define _INC_UI_MOUSE_H_

#include <inc/ui/drawable.h>

namespace ui {

class Mouse : public Drawable {
    public:
        Mouse();
        void moveMouse(int dx, int dy);
};

}

#endif
