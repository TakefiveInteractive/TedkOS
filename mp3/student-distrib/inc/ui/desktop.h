#ifndef _INC_UI_DESKTOP_H_
#define _INC_UI_DESKTOP_H_

#include <inc/ui/drawable.h>

namespace ui {

class Desktop : public Drawable {

    public:
        Desktop();
        virtual const char * getDescription() const override { return "Desktop"; }
};

}

#endif
