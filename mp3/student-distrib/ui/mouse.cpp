#include <inc/ui/compositor.h>
#include <inc/ui/mouse.h>
#include <inc/drivers/mouse.h>
#include <inc/klibs/palloc.h>
#include <inc/fs/filesystem.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/klibs/AutoSpinLock.h>

using namespace filesystem;

namespace ui {

Mouse *mouseObj;

constexpr int MouseWidth = 30;
constexpr int MouseHeight = 44;

constexpr int ClickPointOffsetX = 6;
constexpr int ClickPointOffsetY = 8;

void moveOurMouse(int dx, int dy)
{
    mouseObj->moveMouse(dx, dy);
}

void leftClickHandler()
{
    mouseObj->leftClick();
}

void leftReleaseHandler()
{
    mouseObj->leftRelease();
}

void dragHandler()
{
    mouseObj->drag();
}

Mouse::Mouse() : Drawable(MouseWidth, MouseHeight, ScreenWidth / 2, ScreenHeight / 2)
{
    File mouseFile;
    constexpr auto size = RGBASize<MouseWidth, MouseHeight>;
    pixelBuffer = new uint8_t[size];
    theDispatcher->open(mouseFile, "mouse.png.conv");
    theDispatcher->read(mouseFile, pixelBuffer, size);
    theDispatcher->close(mouseFile);

    mouseObj = this;
    registerMouseMovementHandler(moveOurMouse);
}

spinlock_t mouse_draw_lock = SPINLOCK_UNLOCKED;

void Mouse::moveMouse(int dx, int dy)
{
    AutoSpinLock lock(&mouse_draw_lock);

    int32_t newX = absX;
    int32_t newY = absY;

    newX += dx;
    newY -= dy;
    if (newX < -ClickPointOffsetX) newX = -ClickPointOffsetX;
    if (newX > ScreenWidth - ClickPointOffsetX) newX = ScreenWidth - ClickPointOffsetX;
    if (newY < -ClickPointOffsetY) newY = -ClickPointOffsetY;
    if (newY > ScreenHeight - ClickPointOffsetY) newY = ScreenHeight - ClickPointOffsetY;

    updateLocation(newX, newY);
}

void Mouse::leftClick()
{
    AutoSpinLock lock(&mouse_draw_lock);

    if (isDragging) return;
    isDragging = Compositor::getInstance()->getElementAtPosition(absX, absY);
    auto elem = +isDragging;
    printf("clicked: %s\n", elem->getDescription());
    dragOffsetX = absX - elem->getAbsX();
    dragOffsetY = absY - elem->getAbsY();
}

void Mouse::leftRelease()
{
    AutoSpinLock lock(&mouse_draw_lock);

    isDragging = Nothing;
}

void Mouse::drag()
{
    AutoSpinLock lock(&mouse_draw_lock);

    if (!isDragging) isDragging = Compositor::getInstance()->getElementAtPosition(absX, absY);
    auto elem = +isDragging;
    printf("drag: %s\n", elem->getDescription());
    if (elem->isMovable())
        elem->updateLocation(absX - dragOffsetX, absY - dragOffsetY);
}

}
