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
        int32_t absX;
        int32_t absY;

        int32_t relX;
        int32_t relY;

        bool visible;
        Deque<Container *> children;
        Container *parent;

    public:

        int32_t getAbsX() const { return absX; }
        int32_t getAbsY() const { return absY; }
        int32_t getWidth() const { return width; }
        int32_t getHeight() const { return height; }

        virtual bool isMovable() const { return false; }
        virtual bool isWindow() const { return false; }
        virtual const char * getDescription() const = 0;

        const Rectangle getBoundingRectangle() const
        {
            return Rectangle { .x1 = absX, .y1 = absY, .x2 = absX + width, .y2 = absY + height };
        }

        Container(int32_t _width, int32_t _height, int32_t _x, int32_t _y)
            : width(_width), height(_height), visible(false)
        {
            relX = absX = _x;
            relY = absY = _y;
        }

        void updateLocationFollowingParent(int32_t parentAbsX, int32_t parentAbsY)
        {
            absX = parentAbsX + relX;
            absY = parentAbsY + relY;
            for (size_t i = 0; i < children.size(); i++)
            {
                children[i]->updateLocationFollowingParent(absX, absY);
            }
        }

        virtual void updateLocation(int32_t newX, int32_t newY)
        {
            const int32_t oldX = absX;
            const int32_t oldY = absY;

            absX = newX;
            absY = newY;

            relX += newX - oldX;
            relY += newY - oldY;

            // recursively update child location
            for (size_t i = 0; i < children.size(); i++)
            {
                children[i]->updateLocationFollowingParent(absX, absY);
            }

            // erase old drawable
            const Rectangle & oldRectangle = Rectangle {
                .x1 = oldX,
                .y1 = oldY,
                .x2 = oldX + width,
                .y2 = oldY + height
            };
            Compositor::getInstance()->redraw(oldRectangle);
            // draw ourself on new location
            Compositor::getInstance()->drawSingle(this, getBoundingRectangle(), oldRectangle);
        }

        virtual bool isDrawable() const { return false; }

        bool isPixelInRange(int32_t tx, int32_t ty) const
        {
            if (tx < absX) return false;
            if (tx >= absX + width) return false;
            if (ty < absY) return false;
            if (ty >= absY + height) return false;
            return true;
        }

        void show()
        {
            visible = true;
            Compositor::getInstance()->redraw(getBoundingRectangle());
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

        virtual void setParent(Container *_parent)
        {
            parent = _parent;
            absX = parent->getAbsX() + relX;
            absY = parent->getAbsY() + relY;
        }

        virtual void addChild(Container *d)
        {
            children.push_back(d);
            d->setParent(this);
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
