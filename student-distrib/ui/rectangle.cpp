#include <inc/ui/rectangle.h>

namespace ui {

int32_t Rectangle::BoundX(int32_t input)
{
    return input < 0 ? 0 : input > ScreenWidth ? ScreenWidth : input;
}

int32_t Rectangle::BoundY(int32_t input)
{
    return input < 0 ? 0 : input > ScreenHeight ? ScreenHeight : input;
}

const Rectangle Rectangle::bound() const
{
    return Rectangle { .x1 = BoundX(x1), .y1 = BoundY(y1), .x2 = BoundX(x2), .y2 = BoundY(y2) };
}

bool Rectangle::hasPoint(int32_t x, int32_t y) const
{
    return x >= x1 && x <= x2 && y >= y1 && y <= y2;
}

bool Rectangle::overlapsWith(const Rectangle& r2) const
{
    if (x1 > r2.x2 || r2.x1 > x2) return false;
    if (y1 > r2.y2 || r2.y1 > y2) return false;
    return true;
}

}
