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
};

}   // namespace ui

#endif
