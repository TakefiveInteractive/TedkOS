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

void moveOurMouse(int dx, int dy)
{
    mouseObj->moveMouse(dx, dy);
}

Mouse::Mouse() : Drawable(30, 44, 512, 384)
{
    File mouseFile;
    pixelBuffer = new uint8_t[5280];
    theDispatcher->open(mouseFile, "mouse.png.conv");
    theDispatcher->read(mouseFile, pixelBuffer, 5280);
    theDispatcher->close(mouseFile);

    mouseObj = this;
    registerMouseMovementHandler(moveOurMouse);
}

spinlock_t mouse_draw_lock = SPINLOCK_UNLOCKED;

void Mouse::moveMouse(int dx, int dy)
{
    AutoSpinLock lock(&mouse_draw_lock);

    int32_t newX = x;
    int32_t newY = y;

    newX += dx;
    newY -= dy;
    if (newX < 0) newX = 0;
    if (newX > 1024 - 30) newX = 1024 - 30;
    if (newY < 0) newY = 0;
    if (newY > 768 - 44) newY = 768 - 44;

    updateLocation(newX, newY);
}

}
