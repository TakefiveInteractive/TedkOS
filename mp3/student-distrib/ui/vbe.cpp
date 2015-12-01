#include <inc/x86/paging.h>
#include <inc/ui/vbe.h>
#include <inc/klibs/lib.h>

// According to inc/x86/real.h: If you want to use %es, use REAL_MODE_FREE_SEG

namespace vbe
{
    RawVbeInfoBlock* getVbeInfo()
    {
        real_context_t context;

        // Set output at physical address = REAL_MODE_FREE_SEG << 4 + 0
        context.es = REAL_MODE_FREE_SEG;
        context.di = 0;

        context.ax = 0x4f00;

        legacyInt(0x10, context);

        if(context.ax != 0x004f)
            return NULL;

        return (RawVbeInfoBlock*) RealModePtr(REAL_MODE_FREE_SEG, 0).get32();
    }

    RawVbeVideoModeInfo* getVideoModeInfo(uint16_t mode)
    {
        real_context_t context;

        // Set output at physical address = REAL_MODE_FREE_SEG << 4 + 0
        context.es = REAL_MODE_FREE_SEG;
        context.di = 0;

        context.ax = 0x4f01;
        context.cx = mode;

        legacyInt(0x10, context);

        if(context.ax != 0x004f)
            return NULL;

        return (RawVbeVideoModeInfo*) RealModePtr(REAL_MODE_FREE_SEG, 0).get32();
    }

    Maybe<VideoModeInfo> findVideoModeInfo(function<bool (VideoModeInfo)> predicate)
    {
        // Iterate over all video modes and return the first one which satisfies the predicate
        auto rawVbeInfo = getVbeInfo();
        if (rawVbeInfo)
        {
            VbeInfo vbeInfo(*rawVbeInfo);
            for (size_t i = 0; i < vbeInfo.numModes; i++)
            {
                auto rawVbeInfoMaybe = getVideoModeInfo(vbeInfo.modeList[i]);
                if (rawVbeInfoMaybe)
                {
                    VideoModeInfo videoInfo(*rawVbeInfoMaybe);
                    printf("Mode 0x %x RGB = %s BPP = %d\n", vbeInfo.modeList[i], videoInfo.RGBMask, videoInfo.bitsPerPixel);
                    if (predicate(videoInfo)) return videoInfo;
                }
            }
        }
        return Nothing;
    }

    VbeInfo::VbeInfo(const VbeInfo& other)
    {
        // This is private to ban copying.
    }
    const VbeInfo& VbeInfo::operator = (const VbeInfo& other)
    {
        // This is private to ban copying.
        return *this;
    }

    // classes
    VbeInfo::VbeInfo(const RawVbeInfoBlock& raw)
    {
        vbe2 = raw.VbeVersion >= 0x0200;
        vbe3 = raw.VbeVersion >= 0x0300;

        totalMemory = raw.TotalMemory;
        capabilityFlags = raw.CapabilityFlags;

        char* rawOEMString = (char*)RealModePtr(raw.OemStringPtr).get32();
        auto len = strlen(rawOEMString);
        oemString = new char[len+1];
        oemString[len] = '\0';
        memcpy(oemString, rawOEMString, sizeof(char)*len);

        uint16_t* modeList = (uint16_t*)RealModePtr(raw.VideoModePtr).get32();
        volatile uint32_t numModes = 0;
        for(; modeList[numModes] != 0xffff; numModes++)
            ;
        this->modeList = new uint16_t[numModes+1];
        this->modeList[numModes] = 0xffff;
        this->numModes = numModes;
        for(uint32_t i=0; i<numModes; i++)
            this->modeList[i] = modeList[i];
    }
    VbeInfo::~VbeInfo()
    {
        delete[] oemString;
        delete[] modeList;
    }

    VideoModeInfo::VideoModeInfo(const RawVbeVideoModeInfo& raw)
    {
        xRes = raw.XRes;
        yRes = raw.YRes;
        numPlanes = raw.NumPlanes;
        numImagePages = raw.NumImagePages;
        physBase = raw.PhysBase;
        bitsPerPixel = raw.BitsPerPixel;
        memset(RGBMask, 0, sizeof(RGBMask));

        uint32_t i;
        for(i = 0; i < raw.RedMaskSize; i++)
            RGBMask[i + raw.RedFieldPosition] = 'r';
        for(i = 0; i < raw.GreenMaskSize; i++)
            RGBMask[i + raw.GreenFieldPosition] = 'g';
        for(i = 0; i < raw.BlueMaskSize; i++)
            RGBMask[i + raw.BlueFieldPosition] = 'b';
        for(i = 0; i < raw.RsvMaskSize; i++)
            RGBMask[i + raw.RsvFieldPosition] = '_';
    }

    VideoModeInfo::VideoModeInfo(const VideoModeInfo& other)
    {
        memcpy(this, &other, sizeof(VideoModeInfo));
    }

    VideoModeInfo& VideoModeInfo::operator= (const VideoModeInfo& other)
    {
        memcpy(this, &other, sizeof(VideoModeInfo));
        return *this;
    }

    VideoModeInfo::~VideoModeInfo()
    {
    }

    // ---------- VBEMemHelp : subclasses' CONSTRUCTORS
    VBEMemHelp::VBEMemHelp(const VideoModeInfo& _info, uint8_t* vmem) : info(_info)
    {
        pixelSize = info.bitsPerPixel >> 3;
        totalSize = (size_t)pixelSize * info.xRes * info.yRes;
        this->vmem = vmem;
    }
    VBEMemHelpFactory::HelpSlow::HelpSlow(const VideoModeInfo& _info, uint8_t* vmem) : VBEMemHelp(_info, vmem)
    {
        rCode = colorCode('r');
        gCode = colorCode('g');
        bCode = colorCode('b');
        oCode = colorCode('_');

        for(int i=0; i<4; i++)
            maskCode[i] = colorCode(info.RGBMask[i * 8]);
    }
    VBEMemHelpFactory::HelpRGB ::HelpRGB (const VideoModeInfo& _info, uint8_t* vmem) : VBEMemHelp(_info, vmem) {}
    VBEMemHelpFactory::HelpBGR ::HelpBGR (const VideoModeInfo& _info, uint8_t* vmem) : VBEMemHelp(_info, vmem) {}
    VBEMemHelpFactory::HelpRGBO::HelpRGBO(const VideoModeInfo& _info, uint8_t* vmem) : VBEMemHelp(_info, vmem) {}
    VBEMemHelpFactory::HelpBGRO::HelpBGRO(const VideoModeInfo& _info, uint8_t* vmem) : VBEMemHelp(_info, vmem) {}

    // ---------- VBEMemHelp : subclasses' DESTRUCTORS
    VBEMemHelp::~VBEMemHelp() {}
    VBEMemHelpFactory::HelpSlow::~HelpSlow() {}
    VBEMemHelpFactory::HelpRGB ::~HelpRGB () {}
    VBEMemHelpFactory::HelpBGR ::~HelpBGR () {}
    VBEMemHelpFactory::HelpRGBO::~HelpRGBO() {}
    VBEMemHelpFactory::HelpBGRO::~HelpBGRO() {}

    // ---------- VBEMemHelp : base class functions

    VBEMemHelp* VBEMemHelp::cls(uint8_t val)
    {
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "movl %1, %%eax                                         ;"
            "rep stosb    # reset ECX *byte*                        "
            : /* no outputs */
            : "rg" (totalSize),
              "rg" (val),
              "D" (vmem)
            : "cc", "memory", "ecx", "eax"
        );
        return this;
    }

    VBEMemHelp* VBEMemHelp::copy(uint8_t* buildBuffer)
    {
        const size_t width = info.xRes;
        const size_t height = info.yRes;
        for (size_t y = 0; y < height; y++) for (size_t x = 0; x < width; x++)
            put(x, y, buildBuffer[(x + y * width) * 4 + 0], buildBuffer[(x + y * width) * 4 + 1], buildBuffer[(x + y * width) * 4 + 2]);
        return this;
    }

    // -------- Help Slow

    uint8_t VBEMemHelpFactory::HelpSlow::colorCode(char colorCharName)
    {
        switch (colorCharName)
        {
        case 'r': return 1;
        case 'g': return 2;
        case 'b': return 3;
        default: return 0;
        }
    }

    VBEMemHelp* VBEMemHelpFactory::HelpSlow::put(size_t x, size_t y, uint8_t red, uint8_t green, uint8_t blue)
    {
        size_t offset = (x + y * info.xRes) * pixelSize;
        uint8_t colorByCode[4];
        colorByCode[rCode] = red;
        colorByCode[gCode] = green;
        colorByCode[bCode] = blue;
        colorByCode[oCode] = 0;

        for(int i=0; i<pixelSize; i++)
            vmem[offset + i] = colorByCode[maskCode[i]];

        return (VBEMemHelp*)this;
    }

    // -------- Help RGB

    VBEMemHelp* VBEMemHelpFactory::HelpRGB::put(size_t x, size_t y, uint8_t red, uint8_t green, uint8_t blue)
    {
        const size_t offset = (x + y * info.xRes) * 3;
        vmem[offset + 0] = red;
        vmem[offset + 1] = green;
        vmem[offset + 2] = blue;
        return (VBEMemHelp*)this;
    }

    VBEMemHelp* VBEMemHelpFactory::HelpRGB::copy(uint8_t* buildBuffer)
    {
        const size_t totalPixels = info.xRes * info.yRes;
        uint8_t* writer = vmem;
        for(size_t i=0; i < totalPixels; i++)
        {
            writer[0] = buildBuffer[0];
            writer[1] = buildBuffer[1];
            writer[2] = buildBuffer[2];
            writer += 3;
            buildBuffer += 4;
        }
        return (VBEMemHelp*)this;
    }

    // -------- Help BGR

    VBEMemHelp* VBEMemHelpFactory::HelpBGR::put(size_t x, size_t y, uint8_t red, uint8_t green, uint8_t blue)
    {
        const size_t offset = (x + y * info.xRes) * 3;
        vmem[offset + 0] = blue;
        vmem[offset + 1] = green;
        vmem[offset + 2] = red;
        return (VBEMemHelp*)this;
    }

    VBEMemHelp* VBEMemHelpFactory::HelpBGR::copy(uint8_t* buildBuffer)
    {
        register const size_t totalPixels = info.xRes * info.yRes;
        uint8_t* writer = vmem;
        for(size_t i=0; i < totalPixels; i++)
        {
            writer[2] = buildBuffer[0];
            writer[1] = buildBuffer[1];
            writer[0] = buildBuffer[2];
            writer += 3;
            buildBuffer += 4;
        }
        return (VBEMemHelp*)this;
    }

    // -------- Help RGBO

    VBEMemHelp* VBEMemHelpFactory::HelpRGBO::put(size_t x, size_t y, uint8_t red, uint8_t green, uint8_t blue)
    {
        size_t offset = (x + y * info.xRes) * 4;
        vmem[offset + 0] = red;
        vmem[offset + 1] = green;
        vmem[offset + 2] = blue;
        return (VBEMemHelp*)this;
    }

    VBEMemHelp* VBEMemHelpFactory::HelpRGBO::copy(uint8_t* buildBuffer)
    {
        const size_t totalPixels = info.xRes * info.yRes;
        uint8_t* writer = vmem;
        for(size_t i=0; i < totalPixels; i++)
        {
            writer[0] = buildBuffer[0];
            writer[1] = buildBuffer[1];
            writer[2] = buildBuffer[2];
            writer += 4;
            buildBuffer += 4;
        }
        return (VBEMemHelp*)this;
    }

    // -------- Help BGRO

    VBEMemHelp* VBEMemHelpFactory::HelpBGRO::put(size_t x, size_t y, uint8_t red, uint8_t green, uint8_t blue)
    {
        const size_t offset = (x + y * info.xRes) * 4;
        vmem[offset + 0] = blue;
        vmem[offset + 1] = green;
        vmem[offset + 2] = red;
        return (VBEMemHelp*)this;
    }

    VBEMemHelp* VBEMemHelpFactory::HelpBGRO::copy(uint8_t* buildBuffer)
    {
        const size_t totalPixels = info.xRes * info.yRes;
        uint8_t* writer = vmem;
        for(size_t i=0; i < totalPixels; i++)
        {
            writer[2] = buildBuffer[0];
            writer[1] = buildBuffer[1];
            writer[0] = buildBuffer[2];
            writer += 4;
            buildBuffer += 4;
        }
        return (VBEMemHelp*)this;
    }

    VBEMemHelp* VBEMemHelpFactory::getInstance(const VideoModeInfo& _info, uint8_t* vmem)
    {
        if(_info.RGBMask[0] == 'r' && _info.RGBMask[1] == 'g' && _info.RGBMask[2] == 'b')
        {
            if(_info.bitsPerPixel == 24)
                return (VBEMemHelp*)new VBEMemHelpFactory::HelpRGB(_info, vmem);
            else if(_info.bitsPerPixel == 32)
                return (VBEMemHelp*)new VBEMemHelpFactory::HelpRGBO(_info, vmem);
        }
        if(_info.RGBMask[0] == 'b' && _info.RGBMask[1] == 'g' && _info.RGBMask[2] == 'r')
        {
            if(_info.bitsPerPixel == 24)
                return (VBEMemHelp*)new VBEMemHelpFactory::HelpBGR(_info, vmem);
            else if(_info.bitsPerPixel == 32)
                return (VBEMemHelp*)new VBEMemHelpFactory::HelpBGRO(_info, vmem);
        }
        return (VBEMemHelp*)new VBEMemHelpFactory::HelpSlow(_info, vmem);
    }
}

