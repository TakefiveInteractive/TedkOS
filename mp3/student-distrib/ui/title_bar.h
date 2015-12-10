#ifndef _INC_TITLE_BAR_H_
#define _INC_TITLE_BAR_H_

#include <inc/ui/drawable.h>

namespace ui {

class TitleBar : public Drawable {
    private:
        enum AssetId
        {
            RedLight = 0,
            YellowLight,
            GreenLight,
            DecoratorLeft,
            DecoratorCenter,
            DecoratorRight
        };

        constexpr static int LightSideLength = 16;
        constexpr static int LightVerticalOffset = 6;
        constexpr static int SideDecoratorWidth = 10;
        constexpr static int DecoratorHeight = 31;

    public:
        TitleBar(int width, int x, int y);

        virtual void updateLocation(int32_t newX, int32_t newY) override
        {
            // we delegate moving to parent
            parent->updateLocation(newX, newY);
        }

        virtual bool isMovable() const { return true; }
        virtual const char * getDescription() const override { return "Title bar"; }

};

}   // namespace ui

#endif
