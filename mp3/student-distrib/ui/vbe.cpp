#include <inc/x86/paging.h>
#include <inc/ui/vbe.h>
#include <inc/klibs/lib.h>

// According to inc/x86/real.h: If you want to use %es, use REAL_MODE_FREE_SEG

namespace vbe
{
    RawVbeInfoBlock* getVbeInfo()
    {
        real_context_t context = { };

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
        real_context_t context = { };

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
                auto rawVbeInfo = getVideoModeInfo(vbeInfo.modeList[i]);
                if (rawVbeInfo)
                {
                    VideoModeInfo videoInfo(*rawVbeInfo, vbeInfo.modeList[i]);
                    printf("Mode 0x%x RGB = %s BPP = %d\n", vbeInfo.modeList[i], videoInfo.RGBMask, videoInfo.bitsPerPixel);
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

    VideoModeInfo::VideoModeInfo(const RawVbeVideoModeInfo& raw, const uint16_t index)
        : _rawMode(raw), _index(index)
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

    // ---------- VBEMemHelp : IMPLEMENTATIONS

    // ---------- BGR

    void BGR_cls(uint8_t* vmem, uint8_t val)
    {
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "movl %1, %%eax                                         ;"
            "rep stosb    # reset ECX *byte*                        "
            : /* no outputs */
            : "rg" (1024 * 768 * 3),
              "rg" (val),
              "D" (vmem)
            : "cc", "memory", "ecx", "eax"
        );
    }

    void BGR_put(uint8_t* vmem, uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue)
    {
        vmem[pixel*3 + 0] = blue;
        vmem[pixel*3 + 1] = green;
        vmem[pixel*3 + 2] = red;
    }

    void BGR_copy(uint8_t* vmem, uint8_t* buildBuffer)
    {
        for(int i=0; i < 1024 * 768; i++)
        {
            vmem[2] = buildBuffer[0];
            vmem[1] = buildBuffer[1];
            vmem[0] = buildBuffer[2];
            vmem += 3;
            buildBuffer += 4;
        }
    }

    void BGR_copyRegion(uint8_t* vmem, uint8_t* buildBuffer, int xfrom, int xto, int yfrom, int yto)
    {
        for(int y=yfrom; y<yto; y++)
        {
            uint8_t* writer = &vmem[3*(xfrom + y*1024)];
            uint8_t* reader = &buildBuffer[4*(xfrom + y*1024)];
            for(int x=xfrom; x<xto; x++)
            {
                writer[2] = reader[0];
                writer[1] = reader[1];
                writer[0] = reader[2];
                writer += 3;
                reader += 4;
            }
        }
    }

    // ---------- RGBO

    void RGBO_cls(uint8_t* vmem, uint8_t val)
    {
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "movl %1, %%eax                                         ;"
            "rep stosb    # reset ECX *byte*                        "
            : /* no outputs */
            : "rg" (1024 * 768 * 4),
              "rg" (val),
              "D" (vmem)
            : "cc", "memory", "ecx", "eax"
        );
    }

    void RGBO_put(uint8_t* vmem, uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue)
    {
        vmem[pixel*4 + 0] = blue;
        vmem[pixel*4 + 1] = green;
        vmem[pixel*4 + 2] = red;
    }

    void RGBO_copy(uint8_t* vmem, uint8_t* buildBuffer)
    {
        asm volatile (
            "cld                                                    ;"
            "movl %0, %%ecx                                         ;"
            "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
            : /* no outputs */
            : "i" (1024 * 768 * 4 / 4),
              "S" (buildBuffer),
              "D" (vmem)
            : "cc", "memory", "ecx"
        );
    }

    void RGBO_copyRegion(uint8_t* vmem, uint8_t* buildBuffer, int xfrom, int xto, int yfrom, int yto)
    {
        for(int y=yfrom; y<yto; y++)
        {
            uint8_t* writer = &vmem[4*(xfrom + y*1024)];
            uint8_t* reader = &buildBuffer[4*(xfrom + y*1024)];
            asm volatile (
                "cld                                                    ;"
                "rep movsd    # copy ECX *dword* from M[ESI] to M[EDI]  "
                : /* no outputs */
                : "ecx" (xto - xfrom),
                  "S" (reader),
                  "D" (writer)
                : "cc", "memory"
            );
        }
    }

    // --------- VBEMemHelp objects
    VBEMemHelp HelpRGBO = {
        .put        = RGBO_put,
        .cls        = RGBO_cls,
        .copy       = RGBO_copy,
        .copyRegion = RGBO_copyRegion
    };

    VBEMemHelp HelpBGR  = {
        .put        = BGR_put,
        .cls        = BGR_cls,
        .copy       = BGR_copy,
        .copyRegion = BGR_copyRegion
    };

    Maybe<VBEMemHelp> getMemHelp(const VideoModeInfo& _info)
    {
        if(_info.RGBMask[0] == 'r' && _info.RGBMask[8] == 'g' && _info.RGBMask[16] == 'b')
        {
            if(_info.bitsPerPixel == 32)
                return HelpRGBO;
        }
        if(_info.RGBMask[0] == 'b' && _info.RGBMask[8] == 'g' && _info.RGBMask[16] == 'r')
        {
            if(_info.bitsPerPixel == 24)
                return HelpBGR;
        }
        return Nothing;
    }
}

