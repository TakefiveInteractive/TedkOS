#include <inc/ui/window.h>

namespace ui {

Window::Window(int width, int height, int x, int y) : Container(width, height, x, y)
{
    bar = new TitleBar(width, 0, 0);
    body = new WindowBody(width - 4, height - bar->getHeight() - 4, 2, bar->getHeight() - 4);

    addChild(body);
    addChild(bar);

    body->show();
    bar->show();
}

WindowBody::WindowBody(int width, int height, int x, int y)
    : Drawable(width, height, x, y)
{
    auto size = CalcRGBASize(width, height);
    pixelBuffer = new uint8_t[size];
    memset(pixelBuffer, 0xff, size);
}

}   // namespace ui

