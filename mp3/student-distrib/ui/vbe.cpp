#include <inc/x86/paging.h>
#include <inc/ui/vbe.h>
#include <inc/klibs/lib.h>

// According to inc/x86/real.h: If you want to use %es, use REAL_MODE_FREE_SEG

namespace vbe
{
    Maybe<RawVbeInfoBlock> getVbeInfo()
    {
        real_context_t context;

        // Set output at physical address = REAL_MODE_FREE_SEG << 4 + 0
        context.es = REAL_MODE_FREE_SEG;
        context.di = 0;

        context.ax = 0x4f00;

        legacyInt(0x10, context);

        if(context.ax != 0x004f)
            return Maybe<RawVbeInfoBlock>();

        return Maybe<RawVbeInfoBlock>(*(RawVbeInfoBlock*)RealModePtr(REAL_MODE_FREE_SEG, 0).get32());
    }

    Maybe<RawVbeVideoModeInfo> getVideoModeInfo(uint16_t mode)
    {
        real_context_t context;

        // Set output at physical address = REAL_MODE_FREE_SEG << 4 + 0
        context.es = REAL_MODE_FREE_SEG;
        context.di = 0;

        context.ax = 0x4f01;
        context.cx = mode;

        legacyInt(0x10, context);

        if(context.ax != 0x004f)
            return Maybe<RawVbeVideoModeInfo>();

        return Maybe<RawVbeVideoModeInfo>(*(RawVbeVideoModeInfo*)RealModePtr(REAL_MODE_FREE_SEG, 0).get32());
    }

    // classes
    VbeInfo::VbeInfo(const VbeInfo& other)
    {
        // This is private to ban copying.
    }
    const VbeInfo& VbeInfo::operator = (const VbeInfo& other)
    {
        // This is private to ban copying.
        return *this;
    }
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
        for(uint32_t i=0; i<numModes; i++)
            this->modeList[i] = modeList[i];
    }
    VbeInfo::~VbeInfo()
    {
        delete[] oemString;
        delete[] modeList;
    }

    VideoModeInfo::VideoModeInfo(const VideoModeInfo& other)
    {
        // This is private to ban copying.
    }
    const VideoModeInfo& VideoModeInfo::operator = (const VideoModeInfo& other)
    {
        // This is private to ban copying.
        return *this;
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
    VideoModeInfo::~VideoModeInfo()
    {
    }
}

