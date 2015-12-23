#ifndef _INC_COMPOSITOR_H_
#define _INC_COMPOSITOR_H_

#include <stdint.h>
#include <stddef.h>
#include <inc/x86/real.h>
#include <inc/ui/vbe.h>
#include <inc/ui/rectangle.h>
#include <inc/klibs/maybe.h>
#include <inc/drivers/ievent.h>
#include <inc/drivers/kkc.h>

using vbe::VBEMemHelp;

namespace ui {

class Drawable;
class Container;
class Window;
class Desktop;

enum DisplayMode {
    Video,
    Text
};

constexpr Rectangle EmptyRectangle(-1000, -1000, -2000, -2000);

typedef uint8_t (RGBAGroup)[4];
typedef uint8_t (RGBGroup)[3];
typedef RGBAGroup (PixelRow)[ScreenWidth];
typedef RGBGroup (RGBPixelRow)[ScreenWidth];

class Compositor : public KeyB::IEvent {
    private:
        real_context_t real_context;

        uint16_t originalVideoModeIndex;
        uint16_t videoModeIndex;

        RGBPixelRow *buildBuffer;
        uint8_t* videoMemory;
        DisplayMode displayMode;

        Maybe<vbe::VideoModeInfo> infoMaybe;

        VBEMemHelp drawHelper;

        // Compositor singleton object
        static Compositor *comp;
        Compositor();

        void enterVideoMode();
        void enterTextMode();

    public:
        static Compositor* getInstance();

        void drawNikita();

        Desktop *rootContainer;

        // redraw entire rectangular area (costly!)
        void redraw(const Rectangle &rect);
        void redraw(const Rectangle &_rect, const Rectangle &_diff);

        // draw single element
        void drawSingle(const Container *d, const Rectangle &rect);
        void drawSingle(const Container *d, const Rectangle &rect, const Rectangle &difference);

        Drawable* addText(int txtX, int txtY, char c);

        Container *getElementAtPosition(int absX, int absY);

        // Unit: text font width
        int32_t txtX = 0;

    public:
        virtual void key(uint32_t kkc, bool capslock);

        // Down and Up cuts changes to ONE single key at a time.
        virtual void keyDown(uint32_t kkc, bool capslock)
        {
        }

        virtual void keyUp(uint32_t kkc, bool capslock)
        {
        }

        virtual void show()
        {
            enterVideoMode();
        }

        virtual void hide()
        {
            enterTextMode();
        }
};

float alphaBlending(float p1, float p2, float alpha);

}

#endif
