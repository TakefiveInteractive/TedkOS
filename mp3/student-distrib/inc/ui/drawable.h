#ifndef _INC_UI_DRAWABLE_H_
#define _INC_UI_DRAWABLE_H_

#include <inc/ui/compositor.h>

namespace ui {

// Whenever a drawable is moved around, it should trigger redraw on every pixel
// in its old location, and draw itself onto its new location.
class Drawable {
    protected:

        int32_t width;
        int32_t height;

        // coordinates of the top left corner
        int32_t x;
        int32_t y;

        Drawable(int32_t _width, int32_t _height, int32_t _x, int32_t _y) : width(_width), height(_height), x(_x), y(_y)
        {
        }

        // RGBA layout
        uint8_t *pixelBuffer;

        void updateLocation(int32_t newX, int32_t newY)
        {
            const int32_t oldX = x;
            const int32_t oldY = y;

            x = newX;
            y = newY;

            // erase old drawable
            Compositor::getInstance()->redraw(Rectangle { .x1 = oldX, .y1 = oldY, .x2 = oldX + width, .y2 = oldY + height });

            Compositor::getInstance()->drawSingle(this, Rectangle { .x1 = x, .y1 = y, .x2 = x + width, .y2 = y + height });
        }

    public:

        int32_t getX() const { return x; }
        int32_t getY() const { return y; }

        bool isPixelInRange(int32_t tx, int32_t ty)
        {
            if (tx < x) return false;
            if (tx >= x + width) return false;
            if (ty < y) return false;
            if (ty >= y + height) return false;
            return true;
        }

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
