#include <inc/ui/compositor.h>
#include <inc/ui/drawable.h>
#include <inc/ui/desktop.h>
#include <inc/ui/mouse.h>
#include <inc/ui/vbe.h>
#include <inc/x86/err_handler.h>
#include <inc/x86/idt_init.h>
#include <inc/klibs/palloc.h>

using namespace vbe;
using namespace palloc;

namespace ui {

void paint_screen(VideoModeInfo info, uint8_t *pixel, uint8_t *source)
{
    VBEMemHelp helper(info, pixel);
    const size_t width = info.xRes;
    const size_t height = info.yRes;
    for (size_t y = 0; y < height; y++) for (size_t x = 0; x < width; x++)
    {
        helper.put(x, y, source[(x + y * width) * 4 + 0], source[(x + y * width) * 4 + 1], source[(x + y * width) * 4 + 2]);
    }
}

Compositor* Compositor::comp = nullptr;

Compositor* Compositor::getInstance()
{
    if (comp) return comp;
    comp = new Compositor();
    return comp;
}

Compositor::Compositor() : numDrawables(0)
{
    runWithoutNMI([this] () {
        infoMaybe = findVideoModeInfo([](VideoModeInfo& modeInfo) {
            // Locate 1024x768x24 mode
            if (
                modeInfo.xRes == 1024 &&
                modeInfo.yRes == 768 &&
                 (modeInfo.bitsPerPixel == 24 ||
                  modeInfo.bitsPerPixel == 32)
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
        if (!infoMaybe)
        {
            printf("1024*768 24bits mode is NOT supported.\n");
            trigger_exception<27>();
        }
        else printf("RGBMasks = %s \n", (+infoMaybe).RGBMask);
        VideoModeInfo mode = +infoMaybe;
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

        drawHelper = new VBEMemHelp(mode, videoMemory);
        drawables = new Drawable*[5];
    });
}

void Compositor::addDrawable(Drawable *d)
{
    drawables[numDrawables] = d;
    numDrawables++;
}

float alphaBlending(float p1, float p2, float alpha)
{
    return p1 * (1.0F - alpha) + p2 * alpha;
}

void Compositor::redraw(const Rectangle &rect)
{
    for (int32_t y = rect.y1; y < rect.y2; y++)
    {
        for (int32_t x = rect.x1; x < rect.x2; x++)
        {
            // Fill it with black ink
            float r = 0.0F;
            float g = 0.0F;
            float b = 0.0F;
            // Draw all drawables
            for (int32_t i = 0; i < numDrawables; i++)
            {
                if (drawables[i]->isPixelInRange(x, y))
                {
                    int32_t relX = x - drawables[i]->getX(), relY = y - drawables[i]->getY();
                    const float alpha = drawables[i]->getAlpha(relX, relY) / 256.0F;
                    r = alphaBlending(r, drawables[i]->getRed(relX, relY), alpha);
                    g = alphaBlending(g, drawables[i]->getGreen(relX, relY), alpha);
                    b = alphaBlending(b, drawables[i]->getBlue(relX, relY), alpha);
                }
            }
            drawHelper->put(x, y, r, g, b);
        }
    }
}

void Compositor::drawSingle(Drawable *d, const Rectangle &rect)
{
    for (int32_t y = rect.y1; y < rect.y2; y++)
    {
        for (int32_t x = rect.x1; x < rect.x2; x++)
        {
            uint8_t r, g, b;
            drawHelper->get(x, y, &r, &g, &b);

            int32_t relX = x - d->getX(), relY = y - d->getY();
            const float alpha = d->getAlpha(relX, relY) / 256.0F;
            r = alphaBlending(r, d->getRed(relX, relY), alpha);
            g = alphaBlending(g, d->getGreen(relX, relY), alpha);
            b = alphaBlending(b, d->getBlue(relX, relY), alpha);
            drawHelper->put(x, y, r, g, b);
        }
    }
}

void Compositor::drawNikita()
{
    addDrawable(new Desktop());
    addDrawable(new Mouse());

    redraw(Rectangle { .x1 = 0, .y1 = 0, .x2 = 1024, .y2 = 768 });
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
