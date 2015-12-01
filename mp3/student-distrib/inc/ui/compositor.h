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

constexpr int32_t ScreenWidth = 1024;
constexpr int32_t ScreenHeight = 768;

enum VideoMode {
    Video,
    Text
};

struct Rectangle {
    int32_t x1;     // top left
    int32_t y1;
    int32_t x2;     // bottom right
    int32_t y2;
};

typedef uint8_t (RGBAGroup)[4];
typedef RGBAGroup (PixelRow)[ScreenWidth];

class Compositor {
    private:
        uint8_t* nikita = NULL;
        real_context_t real_context;
        uint16_t orig_mode;

        // buildBuffer format: RGBA. + alignment.
        PixelRow *buildBuffer;
        uint8_t* videoMemory;
        VideoMode videoMode;
        uint8_t* mouseImg;

        Maybe<vbe::VideoModeInfo> infoMaybe;

        Drawable **drawables;
        int32_t numDrawables;
        VBEMemHelp drawHelper;

        // Compositor singleton object
        static Compositor *comp;
        Compositor();

    public:
        static Compositor* getInstance();

        void enterVideoMode();
        void enterTextMode();

        void drawNikita();
        void moveMouse(int, int);

        void redraw(const Rectangle &rect);
        void addDrawable(Drawable *d);
        void drawSingle(Drawable *d, const Rectangle &rect);
};

}

#endif
