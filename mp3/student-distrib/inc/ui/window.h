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

        virtual bool isMovable() const { return true; }
        virtual const char * getDescription() const override { return "Window"; }
};

class WindowBody : public Drawable {
    public:
        WindowBody(int width, int height, int x, int y);
        virtual const char * getDescription() const override { return "Window body"; }
};

}   // namespace ui

#endif
