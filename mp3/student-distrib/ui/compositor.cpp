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

void paint_screen(uint8_t *pixel, uint8_t *source)
{
    for (size_t x = 0; x < 1024 * 768; x++)
    {
        pixel[2] = source[0];
        pixel[1] = source[1];
        pixel[0] = source[2];
        pixel += 3;
        source += 4;
    }
}

Compositor::Compositor()
{
    runWithoutNMI([this] () {
        auto VideoMode = findVideoModeInfo([](VideoModeInfo& modeInfo) {
            // Locate 1024x768x24 mode
            if (
                modeInfo.xRes == 1024 &&
                modeInfo.yRes == 768 &&
                modeInfo.bitsPerPixel == 8
                )
            {
                // Ensure there are no reserved bits
                for (size_t i = 32 - 8; i < 33; i++)
                {
                    if (modeInfo.RGBMask[i] != '\0') return false;
                }
                return true;
            }
            return false;
        });
        if (!VideoMode)
        {
            printf("1024*768 24bits mode is NOT supported.\n");
            trigger_exception<27>();
        }
        VideoModeInfo mode = +VideoMode;
        uint32_t ModeMem = mode.physBase;

        // Back up current mode.
        real_context.ax = 0x0f00;
        legacyInt(0x10, real_context);
        orig_mode = real_context.ax & 0x00ff;

        LOAD_4MB_PAGE(ModeMem >> 22, ModeMem, PG_WRITABLE);
        RELOAD_CR3();

        videoMemory = (uint8_t *) ModeMem;
        // TODO: assuming we are in text mode initially.
        // Figure this out programmatically
        videoMode = Text;
    });
}

void Compositor::drawNikita()
{
    auto physAddr = physPages.allocPage(true);
    if (!physAddr) trigger_exception<27>();

    LOAD_4MB_PAGE(+physAddr, +physAddr << 22, PG_WRITABLE);
    RELOAD_CR3();

    uint8_t *nikita = (uint8_t *)((uint32_t)(+physAddr) << 22);

    File file;
    theDispatcher->open(file, "landscape");
    theDispatcher->read(file, nikita, 1024 * 768 * 4);
    theDispatcher->close(file);

    paint_screen(videoMemory, nikita);
}

void Compositor::enterVideoMode()
{
    runWithoutNMI([this] () {
        if (videoMode == Video) return;
        real_context.ax = 0x4F02;
        real_context.bx = 0x8118;
        legacyInt(0x10, real_context);
        videoMode = Video;
    });
}

void Compositor::enterTextMode()
{
    runWithoutNMI([this] () {
        if (videoMode == Text) return;
        real_context.ax = orig_mode;
        legacyInt(0x10, real_context);
        videoMode = Text;
    });
}

}
