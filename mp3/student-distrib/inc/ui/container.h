#ifndef _INC_UI_CONTAINER_H_
#define _INC_UI_CONTAINER_H_

#include <stddef.h>
#include <stdint.h>
#include <inc/ui/compositor.h>
#include <inc/klibs/deque.h>

namespace ui {

class Container {

    protected:

        int32_t width;
        int32_t height;

        // coordinates of the top left corner
        int32_t x;
        int32_t y;

        bool visible;
        Deque<Container *> children;

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

        virtual bool isDrawable() const { return false; }

        bool isPixelInRange(int32_t tx, int32_t ty) const
        {
            if (tx < x) return false;
            if (tx >= x + width) return false;
            if (ty < y) return false;
            if (ty >= y + height) return false;
            return true;
        }

        void show()
        {
            visible = true;
            Compositor::getInstance()->drawSingle(this, getBoundingRectangle());
        }

        void hide()
        {
            visible = false;
            Compositor::getInstance()->redraw(getBoundingRectangle());
        }

        bool isVisible() const
        {
            return visible;
        }

        virtual void addChild(Container *d)
        {
            children.push_back(d);
            // draw this thing
            Compositor::getInstance()->drawSingle(d, d->getBoundingRectangle());
        }

        virtual const Deque<Container *>& getChildren() const
        {
            return children;
        }
};

}   // namespace ui

#endif
