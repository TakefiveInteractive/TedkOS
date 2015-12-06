#include <inc/ui/compositor.h>
#include <inc/ui/drawable.h>
#include <inc/ui/desktop.h>
#include <inc/ui/mouse.h>
#include <inc/ui/window.h>
#include <inc/ui/vbe.h>
#include <inc/x86/err_handler.h>
#include <inc/x86/idt_init.h>
#include <inc/klibs/palloc.h>
#include <inc/klibs/lib.h>

using namespace vbe;
using namespace palloc;

namespace ui {

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
        auto videoModeMaybe = findVideoModeInfo([](VideoModeInfo& modeInfo) {
            // Locate ScreenWidth x ScreenHeight 8BPP mode
            if (
                modeInfo.xRes == ScreenWidth &&
                modeInfo.yRes == ScreenHeight &&
                modeInfo._rawMode.RedMaskSize == 8 &&
                modeInfo._rawMode.GreenMaskSize == 8 &&
                modeInfo._rawMode.BlueMaskSize == 8
                )
            {
                return true;
            }
            return false;
        });
        if (!videoModeMaybe)
        {
            printf("%d * %d 8BPP mode is NOT supported.\n", ScreenWidth, ScreenHeight);
            trigger_exception<27>();
        }
        VideoModeInfo mode = +videoModeMaybe;
        printf("RGBMasks = %s \n", mode.RGBMask);
        uint32_t modeMem = mode.physBase;
        uint16_t modePageIdx = modeMem >> 22;

        // Back up current mode.
        real_context.ax = 0x0f00;
        legacyInt(0x10, real_context);
        originalVideoModeIndex = real_context.ax & 0x00ff;

        // Obtain target video mode
        videoModeIndex = mode._index;

        PhysAddr physAddr = PhysAddr(modePageIdx, PG_WRITABLE);
        // Don't let others use the physical video memory
        physPages.markPageAsUsed(modePageIdx);
        void *modeMemVirt = +virtLast1G.allocPage(true);
        cpu0_memmap.addCommonPage(modeMemVirt, physAddr);

        videoMemory = (uint8_t *) modeMemVirt;
        // TODO: assuming we are in text mode initially.
        // Figure this out programmatically
        displayMode = Text;

        // TODO: free the pages allocated here.
        buildBuffer = (PixelRow*) +virtLast1G.allocPage(true);
        cpu0_memmap.addCommonPage(VirtAddr(buildBuffer), PhysAddr((+physPages.allocPage(true)), PG_WRITABLE));

        drawHelper = +getMemHelp(mode);
        drawables = new Drawable*[5];

        memset(buildBuffer, 0, RGBASize<ScreenWidth, ScreenHeight>);
        drawHelper.cls(videoMemory, 0);
    });
}

void Compositor::addDrawable(Drawable *d)
{
    drawables[numDrawables] = d;
    numDrawables++;
    // draw this thing
    redraw(d->getBoundingRectangle());
}

float alphaBlending(float p1, float p2, float alpha)
{
    return p1 * (1.0F - alpha) + p2 * alpha;
}

void Compositor::redraw(const Rectangle &_rect)
{
    const Rectangle &rect = _rect.bound();
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
                if (drawables[i]->isVisible() && drawables[i]->isPixelInRange(x, y))
                {
                    int32_t relX = x - drawables[i]->getX(), relY = y - drawables[i]->getY();
                    const float alpha = drawables[i]->getAlpha(relX, relY) / 256.0F;
                    r = alphaBlending(r, drawables[i]->getRed(relX, relY), alpha);
                    g = alphaBlending(g, drawables[i]->getGreen(relX, relY), alpha);
                    b = alphaBlending(b, drawables[i]->getBlue(relX, relY), alpha);
                }
            }
            buildBuffer[y][x][0] = r;
            buildBuffer[y][x][1] = g;
            buildBuffer[y][x][2] = b;
        }
    }
    if (displayMode == Video)
        drawHelper.copyRegion(videoMemory, (uint8_t *)buildBuffer, rect.x1, rect.x2, rect.y1, rect.y2);
}

void Compositor::drawSingle(Drawable *d, const Rectangle &_rect)
{
    if (d->isVisible() == false) return;

    const Rectangle &rect = _rect.bound();
    for (int32_t y = rect.y1; y < rect.y2; y++)
    {
        for (int32_t x = rect.x1; x < rect.x2; x++)
        {
            uint8_t r, g, b;
            r = buildBuffer[y][x][0];
            g = buildBuffer[y][x][1];
            b = buildBuffer[y][x][2];

            int32_t relX = x - d->getX(), relY = y - d->getY();
            const float alpha = d->getAlpha(relX, relY) / 256.0F;
            r = alphaBlending(r, d->getRed(relX, relY), alpha);
            g = alphaBlending(g, d->getGreen(relX, relY), alpha);
            b = alphaBlending(b, d->getBlue(relX, relY), alpha);

            buildBuffer[y][x][0] = r;
            buildBuffer[y][x][1] = g;
            buildBuffer[y][x][2] = b;
        }
    }
    if (displayMode == Video)
        drawHelper.copyRegion(videoMemory, (uint8_t *)buildBuffer, rect.x1, rect.x2, rect.y1, rect.y2);
}

void Compositor::drawNikita()
{
    addDrawable(new Desktop());
    addDrawable(new Mouse());
}

void Compositor::enterVideoMode()
{
    runWithoutNMI([this] () {
        if (displayMode == Video) return;
        real_context.ax = SELECT_VISA_VIDEO_MODE;
        real_context.bx = 0x8000 | videoModeIndex;
        legacyInt(0x10, real_context);
        displayMode = Video;
        // Trigger whole screen update
        redraw(Rectangle { .x1 = 0, .y1 = 0, .x2 = ScreenWidth, .y2 = ScreenHeight });
    });
}

void Compositor::enterTextMode()
{
    runWithoutNMI([this] () {
        if (displayMode == Text) return;
        real_context.ax = originalVideoModeIndex;
        legacyInt(0x10, real_context);
        displayMode = Text;
    });
}

}
