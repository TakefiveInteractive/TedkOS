#ifndef _INC_UI_DESKTOP_H_
#define _INC_UI_DESKTOP_H_

#include <inc/ui/drawable.h>

namespace ui {

class Desktop : public Drawable {

    public:
        Desktop();
        virtual const char * getDescription() const override { return "Desktop"; }
        virtual void addChild(Container *d) override;

        Drawable *theMouse = nullptr;
};

}

#endif
