#ifndef _INC_UI_CONTAINER_H_
#define _INC_UI_CONTAINER_H_

namespace ui {

class Container {

    protected:

        int32_t width;
        int32_t height;

        // coordinates of the top left corner
        int32_t x;
        int32_t y;

        bool visible;

    public:

        int32_t getX() const { return x; }
        int32_t getY() const { return y; }

        const Rectangle getBoundingRectangle() const
        {
            return Rectangle { .x1 = x, .y1 = y, .x2 = x + width, .y2 = y + height };
        }

        Container(int32_t _width, int32_t _height, int32_t _x, int32_t _y)
            : width(_width), height(_height), x(_x), y(_y), visible(true)
        {
        }

        virtual bool isDrawable() override { return false; }
};

}   // namespace ui

#endif
