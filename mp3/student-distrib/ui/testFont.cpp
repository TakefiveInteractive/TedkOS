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

constexpr int numFont = 94;
constexpr int FontWidth = 20;
constexpr int FontHeight = 17;
constexpr int FontCharSpacing = 0;
constexpr int FontLineSpacing = 8;

TestFont::TestFont() : Drawable(FontWidth, FontHeight, 0, 0) {
    File fontFile;
    struct stat st;
    theDispatcher->open(fontFile, "inconsolata_15.carr");
    theDispatcher->fstat(fontFile, &st);
    const uint32_t size = st.st_size;
    auto buffer = new uint8_t[size];
    theDispatcher->read(fontFile, buffer, size);
    ArrFile* fileParser = ArrFile::getInstance((char*) buffer);
    pixelBuffer = (uint8_t*) (*fileParser)[70];
    theDispatcher->close(fontFile);
}

}
