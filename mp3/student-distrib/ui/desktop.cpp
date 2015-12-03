#include <inc/ui/desktop.h>
#include <inc/klibs/palloc.h>
#include <inc/fs/filesystem.h>
#include <inc/syscalls/filesystem_wrapper.h>

using namespace palloc;
using namespace filesystem;

namespace ui {

Desktop::Desktop() : Drawable(ScreenWidth, ScreenHeight, 0, 0)
{
    auto physAddr = physPages.allocPage(true);
    auto virtAddr = virtLast1G.allocPage(true);
    if (!physAddr || !virtAddr) trigger_exception<27>();

    cpu0_memmap.addCommonPage(VirtAddr((+virtAddr)), PhysAddr(+physAddr, PG_WRITABLE));

    pixelBuffer = (uint8_t *)+virtAddr;

    File file;
    theDispatcher->open(file, "landscape");
    theDispatcher->read(file, pixelBuffer, RGBASize<ScreenWidth, ScreenHeight>);
    theDispatcher->close(file);
}

}
