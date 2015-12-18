#include <inc/ui/container.h>
#include <inc/ui/tasklet.h>

namespace ui
{

void Container::updateLocation(int32_t newX, int32_t newY)
{
        const int32_t oldX = absX;
        const int32_t oldY = absY;

        absX = newX;
        absY = newY;

        relX += newX - oldX;
        relY += newY - oldY;

    Tasklet::getInstance()->add([=]()
    {

        // recursively update child location
        for (size_t i = 0; i < children.size(); i++)
        {
            children[i]->updateLocationFollowingParent(absX, absY);
        }

        const Rectangle & oldRectangle = Rectangle {
            .x1 = oldX,
            .y1 = oldY,
            .x2 = oldX + width,
            .y2 = oldY + height
        };

        // erase old drawable
        Compositor::getInstance()->redraw(oldRectangle);

        // There are other tasks
        if(Tasklet::getInstance()->size() > 0)
            return;

        // draw ourself on new location
        Compositor::getInstance()->drawSingle(this, getBoundingRectangle(), oldRectangle);

    });
}

}
