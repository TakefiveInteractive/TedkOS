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
            // Locate 1024x768 8BPP mode
            if (
                modeInfo.xRes == 1024 &&
                modeInfo.yRes == 768 &&
                modeInfo._rawMode.RedMaskSize == 8 &&
                modeInfo._rawMode.GreenMaskSize == 8 &&
                modeInfo._rawMode.BlueMaskSize == 8
                )
            {
                return true;
            }
            return false;
        });
        if (!VideoMode)
        {
            printf("1024*768 8BPP mode is NOT supported.\n");
            trigger_exception<27>();
        }
        VideoModeInfo mode = +VideoMode;
        uint32_t ModeMem = mode.physBase;

        // Back up current mode.
        real_context.ax = 0x0f00;
        legacyInt(0x10, real_context);
        orig_mode = real_context.ax & 0x00ff;

        PhysAddr physAddr = PhysAddr(ModeMem >> 22, PG_WRITABLE);
        cpu0_memmap.addCommonPage(VirtAddr((void*)ModeMem), physAddr);

        RELOAD_CR3();

        videoMemory = (uint8_t *) ModeMem;
        // TODO: assuming we are in text mode initially.
        // Figure this out programmatically
        videoMode = Text;
    });
}

void Compositor::moveMouse(int dx, int dy)
{
    mouseX += dx;
    mouseY -= dy;
    if (mouseX < 0) mouseX = 0;
    if (mouseX > 1024 - 30) mouseX = 1024 - 30;
    if (mouseY < 0) mouseY = 0;
    if (mouseY > 768 - 44) mouseY = 768 - 44;
    for (size_t y = 0; y < 44; y++)
    {
        for (size_t x = 0; x < 30; x++)
        {
            float alpha = (float) mouseImg[(30 * y + x) * 4 + 3] / 256.0F;
            videoMemory[(1024 * (mouseY + y) + mouseX + x) * 3 + 0] = videoMemory[(1024 * (mouseY + y) + mouseX + x) * 3 + 0] * (1.0F - alpha) + alpha * mouseImg[(30 * y + x) * 4 + 2];
            videoMemory[(1024 * (mouseY + y) + mouseX + x) * 3 + 1] = videoMemory[(1024 * (mouseY + y) + mouseX + x) * 3 + 1] * (1.0F - alpha) + alpha * mouseImg[(30 * y + x) * 4 + 1];
            videoMemory[(1024 * (mouseY + y) + mouseX + x) * 3 + 2] = videoMemory[(1024 * (mouseY + y) + mouseX + x) * 3 + 2] * (1.0F - alpha) + alpha * mouseImg[(30 * y + x) * 4 + 0];
        }
    }
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


    File mouseFile;
    mouseImg = new uint8_t[5280];
    theDispatcher->open(mouseFile, "mouse.png.conv");
    theDispatcher->read(mouseFile, mouseImg, 5280);
    theDispatcher->close(mouseFile);

    paint_screen(videoMemory, nikita);
    moveMouse(0, 0);
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
