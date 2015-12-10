#ifndef _INC_COMPOSITOR_H_
#define _INC_COMPOSITOR_H_

#include <stdint.h>
#include <stddef.h>
#include <inc/x86/real.h>
#include <inc/ui/vbe.h>
#include <inc/klibs/maybe.h>

using vbe::VBEMemHelp;

namespace ui {

class Drawable;
class Container;
class Window;

constexpr int32_t ScreenWidth = 1024;
constexpr int32_t ScreenHeight = 768;

template<int32_t width, int32_t height>
constexpr size_t RGBASize = width * height * 4;

constexpr size_t CalcRGBASize(int32_t width, int32_t height) { return width * height * 4; }

enum DisplayMode {
    Video,
    Text
};

struct Rectangle {
    const int32_t x1;     // top left
    const int32_t y1;
    const int32_t x2;     // bottom right
    const int32_t y2;

    constexpr Rectangle(int32_t a, int32_t b, int32_t c, int32_t d)
        : x1(a), y1(b), x2(c), y2(d) { }

    static int32_t BoundX(int32_t input) { return input < 0 ? 0 : input > ScreenWidth ? ScreenWidth : input; }
    static int32_t BoundY(int32_t input) { return input < 0 ? 0 : input > ScreenHeight ? ScreenHeight : input; }

    const Rectangle bound() const {
        return Rectangle { .x1 = BoundX(x1), .y1 = BoundY(y1), .x2 = BoundX(x2), .y2 = BoundY(y2) };
    }

    bool hasPoint(int32_t x, int32_t y) const
    {
        return x >= x1 && x <= x2 && y >= y1 && y <= y2;
    }
};

constexpr Rectangle EmptyRectangle(-1000, -1000, -2000, -2000);

typedef uint8_t (RGBAGroup)[4];
typedef RGBAGroup (PixelRow)[ScreenWidth];

class Compositor {
    private:
        real_context_t real_context;

        uint16_t originalVideoModeIndex;
        uint16_t videoModeIndex;

        // buildBuffer format: RGBA. + alignment.
        PixelRow *buildBuffer;
        uint8_t* videoMemory;
        DisplayMode displayMode;

        Maybe<vbe::VideoModeInfo> infoMaybe;

        Container *rootContainer;
        VBEMemHelp drawHelper;

        // Compositor singleton object
        static Compositor *comp;
        Compositor();

    public:
        static Compositor* getInstance();

        void enterVideoMode();
        void enterTextMode();

        void drawNikita();

        // redraw entire rectangular area (costly!)
        void redraw(const Rectangle &rect);

        // draw single element
        void drawSingle(const Container *d, const Rectangle &rect);
        void drawSingle(const Container *d, const Rectangle &rect, const Rectangle &difference);
};

}

#endif
