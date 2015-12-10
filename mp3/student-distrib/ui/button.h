#ifndef _INC_UI_BUTTON_H
#define _INC_UI_BUTTON_H_

#include <inc/ui/drawable.h>

namespace ui {

class Button : public Drawable {
    public:
        Button(int width, int height, int x, int y);
        virtual const char * getDescription() const override { return "Button"; }
};

}   // namespace ui

#endif
