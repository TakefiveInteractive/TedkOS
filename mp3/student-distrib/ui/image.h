#ifndef _INC_UI_IMAGE_H
#define _INC_UI_IMAGE_H_

#include <inc/ui/drawable.h>

namespace ui {

class Image : public Drawable {
    public:
        Image(int width, int height, int x, int y);
        virtual const char * getDescription() const override { return "Button"; }
};

}   // namespace ui

#endif

