#include <inc/ui/compositor.h>
#include <inc/klibs/arrFile.h>
#include <inc/fs/filesystem.h>
#include "title_bar.h"

using namespace filesystem;

namespace ui {

TitleBar::TitleBar(int width, int x, int y) : Drawable(width, DecoratorHeight, x, y)
{
    File barFile;
    struct stat st;
    theDispatcher->open(barFile, "title_bar.carr");
    theDispatcher->fstat(barFile, &st);
    const uint32_t size = st.st_size;
    auto buffer = new uint8_t[size];
    theDispatcher->read(barFile, buffer, size);
    ArrFile &fileParser = *ArrFile::getInstance((char*) buffer);

    uint8_t *leftDecorator = (uint8_t *) fileParser[DecoratorLeft];
    uint8_t *rightDecorator = (uint8_t *) fileParser[DecoratorRight];
    uint8_t *centerDecorator = (uint8_t *) fileParser[DecoratorCenter];

    // paint pixel buffer
    pixelBuffer = new uint8_t[CalcRGBASize(width, DecoratorHeight)];

    // paint background
    for (int y = 0; y < DecoratorHeight; y++)
    {
        for (int x = 0; x < SideDecoratorWidth; x++)
        {
            uint8_t *pix = pixelBuffer + (y * width + x) * 4;
            uint8_t *srcPix = leftDecorator + (y * SideDecoratorWidth + x) * 4;
            for (int z = 0; z < 4; z++)
            {
                pix[z] = srcPix[z];
            }
        }
        for (int x = 0; x < SideDecoratorWidth; x++)
        {
            uint8_t *pix = pixelBuffer + (y * width + x + (width - SideDecoratorWidth)) * 4;
            uint8_t *srcPix = rightDecorator + (y * SideDecoratorWidth + x) * 4;
            for (int z = 0; z < 4; z++)
            {
                pix[z] = srcPix[z];
            }
        }
        for (int x = 0; x < width - SideDecoratorWidth * 2; x++)
        {
            uint8_t *pix = pixelBuffer + (y * width + x + SideDecoratorWidth) * 4;
            uint8_t *srcPix = centerDecorator + y * 4;
            for (int z = 0; z < 4; z++)
            {
                pix[z] = srcPix[z];
            }
        }
    }

    // paint lights
    constexpr static int LightOffsets[3] = { 10, 34, 58 };
    constexpr static int LightIds[3] = { RedLight, YellowLight, GreenLight };
    uint8_t ** lights = new uint8_t* [3];
    for (int i = 0; i < 3; i++)
    {
        lights[i] = reinterpret_cast<uint8_t *>(fileParser[LightIds[i]]);
    }
    for (int y = 0; y < LightSideLength; y++)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int x = 0; x < LightSideLength; x++)
            {
                uint8_t *pix = pixelBuffer + ((y + LightVerticalOffset) * width + x + LightOffsets[i]) * 4;
                uint8_t *srcPix = lights[i] + (y * LightSideLength + x) * 4;
                for (int z = 0; z < 3; z++)
                {
                    pix[z] = alphaBlending(pix[z], srcPix[z], srcPix[3] / 255.0F);
                }
            }
        }
    }

}

}   // namespace ui
