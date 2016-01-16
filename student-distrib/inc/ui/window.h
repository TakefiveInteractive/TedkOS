#ifndef _INC_UI_WINDOW_H_
#define _INC_UI_WINDOW_H_

#include <inc/ui/container.h>
#include <inc/klibs/holds_process_reference.hpp>

namespace ui {

class WindowBody;
class TitleBar;

class Window : public Container, HoldsProcessReference {
    private:
        TitleBar *bar;
        WindowBody *body;

    public:
        Window(int width, int height, int x, int y);

        virtual bool isMovable() const override { return true; }
        virtual bool isWindow() const override { return true; }
        virtual const char * getDescription() const override { return "Window"; }

        virtual void addChild(Container *d) override;
};

class WindowBody : public Drawable {
    public:
        WindowBody(int width, int height, int x, int y);
        virtual const char * getDescription() const override { return "Window body"; }
};

}   // namespace ui

#endif
