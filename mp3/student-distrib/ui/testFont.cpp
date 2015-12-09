#include <inc/ui/compositor.h>
#include <inc/ui/testFont.h>
#include <inc/drivers/mouse.h>
#include <inc/klibs/palloc.h>
#include <inc/fs/filesystem.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/klibs/AutoSpinLock.h>
#include <inc/klibs/arrFile.h>
#include <inc/fs/stat.h>

using namespace filesystem;

namespace ui {

constexpr int NumFont = 94;
constexpr int FontWidth = 20;
constexpr int FontHeight = 42;
constexpr int FontCharSpacing = 0;
constexpr int FontLineSpacing = 8;

uint8_t *renderRGBAFont(ArrFile &parser, int width, int height)
{
    auto buffer = new uint8_t[CalcRGBASize(width * 40, height)];
    for (int i = 0; i < 40; i++)
    {
        uint8_t *grayScaleFont = (uint8_t*) parser[i + 65];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                uint8_t *pix = buffer + (y * width * 40 + x + i * width) * 4;
                uint8_t *grayPix = grayScaleFont + (y * width + x);
                pix[0] = 0;
                pix[1] = 0;
                pix[2] = 0;
                pix[3] = grayPix[0];
            }
        }
    }
    return buffer;
}

TestFont::TestFont() : Drawable(FontWidth * 40, FontHeight, 0, 0) {
    File fontFile;
    struct stat st;
    theDispatcher->open(fontFile, "inconsolata_36.carr");
    theDispatcher->fstat(fontFile, &st);
    const uint32_t size = st.st_size;
    auto buffer = new uint8_t[size];
    theDispatcher->read(fontFile, buffer, size);
    ArrFile* fileParser = ArrFile::getInstance((char*) buffer);
    pixelBuffer = renderRGBAFont(*fileParser, FontWidth, FontHeight);
    theDispatcher->close(fontFile);
}

}
