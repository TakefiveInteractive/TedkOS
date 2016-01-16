#ifndef _UI_RECTANGLE_H_
#define _UI_RECTANGLE_H_

#include "screen.h"

namespace ui {

struct Rectangle {
    const int32_t x1;     // top left
    const int32_t y1;
    const int32_t x2;     // bottom right
    const int32_t y2;

    constexpr Rectangle(int32_t a, int32_t b, int32_t c, int32_t d)
        : x1(a), y1(b), x2(c), y2(d)
    {
    }

    static int32_t BoundX(int32_t input);
    static int32_t BoundY(int32_t input);

    const Rectangle bound() const;

    bool hasPoint(int32_t x, int32_t y) const;

    bool overlapsWith(const Rectangle& r2) const;
};

}

#endif
