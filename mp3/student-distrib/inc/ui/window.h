#ifndef _INC_UI_WINDOW_H_
#define _INC_UI_WINDOW_H_

#include <inc/ui/container.h>
#include <ui/title_bar.h>

namespace ui {

class WindowBody;

class Window : public Container {
    private:
        TitleBar *bar;
        WindowBody *body;

    public:
        Window(int width, int height, int x, int y);
};

class WindowBody : public Drawable {
    public:
        WindowBody(int width, int height, int x, int y);
};

}   // namespace ui

#endif
