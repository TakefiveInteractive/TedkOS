#ifndef _INC_COMPOSITOR_H_
#define _INC_COMPOSITOR_H_

#include <stdint.h>
#include <stddef.h>
#include <inc/x86/real.h>

namespace ui {

class Drawable {
    private:
        int32_t width;
        int32_t height;

        int32_t x;
        int32_t y;
};

class Compositor {
    private:
        real_context_t real_context;
        uint16_t orig_mode;
        uint8_t *videoMemory;

    public:
        Compositor();
        void enterVideoMode();
};

}

#endif
