#ifndef _INC_UI_DRAWABLE_H_
#define _INC_UI_DRAWABLE_H_

#include <inc/ui/compositor.h>
#include <inc/ui/container.h>

namespace ui {

// Whenever a drawable is moved around, it should trigger redraw on every pixel
// in its old location, and draw itself onto its new location.
class Drawable : public Container {

    public:
        Drawable(int32_t _width, int32_t _height, int32_t _x, int32_t _y)
            : Container(_width, _height, _x, _y)
        {
        }

        virtual bool isDrawable() const override { return true; }

        // RGBA layout
        uint8_t *pixelBuffer;

        uint8_t getRed(int32_t tx, int32_t ty) const
        {
            return pixelBuffer[(ty * width + tx) * 4 + 0];
        }

        uint8_t getGreen(int32_t tx, int32_t ty) const
        {
            return pixelBuffer[(ty * width + tx) * 4 + 1];
        }

        uint8_t getBlue(int32_t tx, int32_t ty) const
        {
            return pixelBuffer[(ty * width + tx) * 4 + 2];
        }

        uint8_t getAlpha(int32_t tx, int32_t ty) const
        {
            return pixelBuffer[(ty * width + tx) * 4 + 3];
        }
};

}

#endif
