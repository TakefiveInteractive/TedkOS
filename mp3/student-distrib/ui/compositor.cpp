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
#include <inc/klibs/stack.h>
#include <inc/proc/tasks.h>
#include <inc/klibs/maybe.h>
#include <inc/drivers/kkc.h>
#include <inc/ui/testFont.h>
#include <inc/fs/filesystem.h>
#include <inc/klibs/arrFile.h>
#include <inc/fs/stat.h>

#include "title_bar.h"
#include "button.h"
#include "image.h"
#include <inc/ui/ui_syscall.h>

using namespace vbe;
using namespace palloc;


namespace syscall {
    bool validUserPointer(const void* ptr);
}

using syscall::validUserPointer;

namespace ui {

Compositor* Compositor::comp = nullptr;

Compositor* Compositor::getInstance()
{
    if (comp) return comp;
    comp = new Compositor();
    return comp;
}

Compositor::Compositor()
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

        memset(buildBuffer, 0, RGBASize<ScreenWidth, ScreenHeight>);
    });
}

float alphaBlending(float p1, float p2, float alpha)
{
    return p1 * (1.0F - alpha) + p2 * alpha;
}

class TreeIterator
{
private:
    util::Stack<const Container *, 20> stack;

public:
    TreeIterator(const Container *root)
    {
        if (root)
            stack.push(root);
    }

    void reinit(const Container *root)
    {
        stack.resetStackPointer();
        if (root)
            stack.push(root);
    }

    const Maybe<const Container *> iterate()
    {
        if (stack.empty()) return Nothing;
        auto c = stack.pop();
        return c;
    }

    void descend(const Container *c)
    {
        auto& children = c->getChildren();
        for (int i = children.size() - 1; i >= 0; i--)
        {
            stack.push(children[i]);
        }
    }
};

Container * Compositor::getElementAtPosition(int absX, int absY)
{
    TreeIterator itr(rootContainer);
    const Container *candidate = nullptr;
    while (auto resMaybe = itr.iterate())
    {
        auto c = +resMaybe;
        if (c == theMouse) continue;        // never select the mouse
        if (c->isPixelInRange(absX, absY))
        {
            candidate = c;
            itr.descend(c);
        }
    }
    return const_cast<Container *>(candidate);
}

void Compositor::redraw(const Rectangle &_rect)
{
    if (rootContainer != nullptr)
        drawSingle(rootContainer, _rect);
}

void Compositor::drawSingle(const Container *d, const Rectangle &_rect)
{
    drawSingle(d, _rect, EmptyRectangle);
}

void Compositor::drawSingle(const Container *d, const Rectangle &_rect, const Rectangle &_diff)
{
    const Rectangle &rect = _rect.bound();
    TreeIterator itr(d);

    // Draw all drawables
    while (auto resMaybe = itr.iterate())
    {
        auto c = +resMaybe;
        if(!c->isVisible())
            continue;
        if(!c->getBoundingRectangle().overlapsWith(rect))
            continue;
        if(c->isDrawable())
        {
            for (int32_t y = rect.y1; y < rect.y2; y++)
            {
                for (int32_t x = rect.x1; x < rect.x2; x++)
                {
                    if (_diff.hasPoint(x, y)) continue;

                    // Fill it with black ink
                    uint8_t r, g, b;
                    r = buildBuffer[y][x][0];
                    g = buildBuffer[y][x][1];
                    b = buildBuffer[y][x][2];

                    if (c->isPixelInRange(x, y))
                    {
                        auto _draw = reinterpret_cast<const Drawable *>(c);
                        int32_t relX = x - _draw->getAbsX();
                        int32_t relY = y - _draw->getAbsY();
                        const float alpha = _draw->getAlpha(relX, relY) / 255.0F;
                        r = alphaBlending(r, _draw->getRed(relX, relY), alpha);
                        g = alphaBlending(g, _draw->getGreen(relX, relY), alpha);
                        b = alphaBlending(b, _draw->getBlue(relX, relY), alpha);
                    }

                    buildBuffer[y][x][0] = r;
                    buildBuffer[y][x][1] = g;
                    buildBuffer[y][x][2] = b;
                }
            }
        }
        itr.descend(c);
    }
    if (displayMode == Video)
        drawHelper.copyRegion(videoMemory, (uint8_t *)buildBuffer, rect.x1, rect.x2, rect.y1, rect.y2);
}

using namespace filesystem;

static uint8_t *renderRGBAFont(ArrFile &parser, int width, int height, char c)
{
    auto buffer = new uint8_t[CalcRGBASize(width * 40, height)];
    uint8_t *grayScaleFont = (uint8_t*) parser[(size_t)c];
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t *pix = buffer + (y * width * 40 + x) * 4;
            uint8_t *grayPix = grayScaleFont + (y * width + x);
            pix[0] = 0;
            pix[1] = 0;
            pix[2] = 0;
            pix[3] = grayPix[0];
        }
    }
    return buffer;
}


Drawable* Compositor::addText(int txtX, int txtY, char c)
{
    constexpr int NumFont = 94;
    constexpr int FontWidth = 20;
    constexpr int FontHeight = 42;
    constexpr int FontCharSpacing = 0;
    constexpr int FontLineSpacing = 8;

    Drawable* draw = new Drawable(20 * 40, 42,txtX * 20, txtY * 42);
    File fontFile;
    struct stat st;
    theDispatcher->open(fontFile, "inconsolata_36.carr");
    theDispatcher->fstat(fontFile, &st);
    const uint32_t size = st.st_size;
    auto buffer = new uint8_t[size];
    theDispatcher->read(fontFile, buffer, size);
    ArrFile* fileParser = ArrFile::getInstance((char*) buffer);
    draw->pixelBuffer = renderRGBAFont(*fileParser, FontWidth, FontHeight, c);
    theDispatcher->close(fontFile);
    rootContainer->addChild(draw);
    draw->show();

    return draw;
}

void Compositor::drawNikita()
{
    rootContainer = new Desktop();
    redraw(rootContainer->getBoundingRectangle());
    auto wind = new Window(300, 300, 50, 50);
    rootContainer->addChild(wind);

    // Mouse is at the top
    theMouse = new Mouse();
    rootContainer->addChild(theMouse);

    rootContainer->show();
    wind->show();
    theMouse->show();
}

void Compositor::enterVideoMode()
{
    runWithoutNMI([this] () {
        if (displayMode == Video) return;
        real_context.ax = SELECT_VISA_VIDEO_MODE;
        real_context.bx = 0x8000 | videoModeIndex;
        legacyInt(0x10, real_context);
        displayMode = Video;
        drawHelper.cls(videoMemory, 0);
        // whole screen update
        drawHelper.copyRegion(videoMemory, (uint8_t *) buildBuffer, 0, ScreenWidth, 0, ScreenHeight);
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

void Compositor::key(uint32_t kkc, bool capslock)
{
    if(kkc &(~KKC_ASCII_MASK))
        return;
    static Drawable* all[40] = {0};
    all[txtX] = addText(txtX++, 0, (char)kkc);
    if(txtX >= 40)
    {
    txtX = 0;
    for(int i=0; i<40; i++)
        (all[i])->hide();
    }
        
}


// SYSCALLS
//

Container* createWindow(int32_t width, int32_t height)
{
    auto wind = new Window(width, height, 150, 150);
    Compositor::getInstance()->rootContainer->addChild(wind);
    getCurrentThreadInfo()->getProcessDesc()->setMainWindow(wind);
    wind->show();
    return wind;
}

int32_t setText(Container *elem, char *text)
{
    if (!validUserPointer(elem)) return -1;
    return 0;
}

int32_t getText(Container *elem, char *buffer)
{
    if (!validUserPointer(elem)) return -1;
    return 0;
}

int32_t showElement(Container *elem)
{
    if (!validUserPointer(elem)) return -1;
    elem->show();
    return 0;
}

int32_t hideElement(Container *elem)
{
    if (!validUserPointer(elem)) return -1;
    elem->hide();
    return 0;
}

Container* createButton(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y)
{
    auto wind = getCurrentThreadInfo()->getProcessDesc()->getMainWindow();
    auto btn = new Button(width, height, pos_x, pos_y);
    wind->addChild(btn);
    return btn;
}

int32_t getMessage(void *msg)
{
    if (!validUserPointer(msg)) return -1;
}

int32_t attacheMessageHandler(Container *elem, void *args)
{
    if (!validUserPointer(elem)) return -1;
}

Container* createImage(int32_t width, int32_t height, int32_t pos_x, int32_t pos_y)
{
    auto wind = getCurrentThreadInfo()->getProcessDesc()->getMainWindow();
    auto img = new Image(width, height, pos_x, pos_y);
    wind->addChild(img);
    return img;
}

int32_t setImageData(Container *img, uint8_t *data)
{
    if (!validUserPointer(img)) return -1;
}


}
