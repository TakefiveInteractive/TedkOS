#ifndef _INC_KB_IEVENT
#define _INC_KB_IEVENT

namespace KeyB {
    // By default, all functions do nothing.
    class IEvent
    {
    public:
        virtual void key(uint32_t kkc, bool capslock) = 0;

        // Down and Up cuts changes to ONE single key at a time.
        virtual void keyDown(uint32_t kkc, bool capslock) = 0;
        virtual void keyUp(uint32_t kkc, bool capslock) = 0;

        virtual void show() = 0;
        virtual void hide() = 0;
    };
}   // namespace KeyB


#endif
