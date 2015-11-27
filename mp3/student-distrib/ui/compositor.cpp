#include <inc/ui/compositor.h>
#include <inc/fs/filesystem.h>
#include <inc/ui/vbe.h>
#include <inc/klibs/palloc.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/x86/err_handler.h>
#include <inc/x86/idt_init.h>

using namespace vbe;
using namespace palloc;
using namespace filesystem;

namespace ui {

Compositor::Compositor()
{
    runWithoutNMI([this] () {
        auto Mode118Maybe = getVideoModeInfo(0x118);
        if (Mode118Maybe)
        {
            // Nothing
        }
        else
        {
            printf("1024*768 24bits mode is NOT supported.\n");
            trigger_exception<27>();
        }
        VideoModeInfo Mode118(+Mode118Maybe);
        uint32_t Mode118Mem = Mode118.physBase;

        // Back up current mode.
        real_context.ax = 0x0f00;
        legacyInt(0x10, real_context);
        orig_mode = real_context.ax & 0x00ff;

        LOAD_4MB_PAGE(Mode118Mem >> 22, Mode118Mem, PG_WRITABLE);
        RELOAD_CR3();

        auto physAddr = physPages.allocPage(true);
        if (!physAddr) trigger_exception<27>();

        LOAD_4MB_PAGE(+physAddr, +physAddr << 22, PG_WRITABLE);
        RELOAD_CR3();

        videoMemory = (uint8_t *) Mode118Mem;
    });
}

void Compositor::enterVideoMode()
{
    runWithoutNMI([this] () {
        real_context.ax = 0x4F02;
        real_context.bx = 0x8118;
        legacyInt(0x10, real_context);
    });
}

}
