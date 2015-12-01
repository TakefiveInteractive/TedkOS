#ifndef _UI_VBE_H
#define _UI_VBE_H

#include <inc/x86/real.h>
#include <inc/klibs/maybe.h>
#include <inc/klibs/function.h>

// References:
// Definition of the following "struct" comes from:
//      OSDev Wiki, and
//      "VBE 2.0 Core functions Standard"

typedef struct {
    char VbeSignature[4];                   // == "VESA"
    uint16_t VbeVersion;                    // == 0x0300 for VBE 3.0
    uint16_t OemStringPtr[2];               // isa vbeFarPtr
    uint32_t CapabilityFlags;
    uint16_t VideoModePtr[2];               // isa vbeFarPtr
    uint16_t TotalMemory;                   // as # of 64KB blocks
    uint8_t reserved_vbe_1_x[236];

    // The following fields are only supported by VBE2.0
    uint16_t VBE2_oem_version;
    uint16_t VendorNamePtr[2];
    uint16_t ProductNamePtr[2];
    uint16_t ProductRevStrPtr[2];
} __attribute__((packed)) RawVbeInfoBlock;

typedef struct  {
    uint16_t Attributes;
    uint8_t  WinAsAttr,WinBsAttr;           // Window A/B's attribute
    uint16_t Granularity;
    uint16_t WinSize;
    uint16_t WinASeg, WinBSeg;              // Window A/B's address (segment part)
    uint16_t WinFuncPtr[2];
    uint16_t BytesPerScanLine;

    uint16_t XRes, YRes;
    uint8_t XCharSize, YCharSize;
    uint8_t NumPlanes, BitsPerPixel, NumBanks;
    uint8_t MemoryModelType, BankSize, NumImagePages;
    uint8_t Reserved0;

    uint8_t RedMaskSize, RedFieldPosition;
    uint8_t GreenMaskSize, GreenFieldPosition;
    uint8_t BlueMaskSize, BlueFieldPosition;
    uint8_t RsvMaskSize, RsvFieldPosition;
    uint8_t DirectColorAttributes;

    uint32_t PhysBase;                      // LFB (Linear Framebuffer) PHYSICAL addr
    uint16_t OffScreenMemPtr[2];
    uint16_t OffScreenMemSize;
} __attribute__((packed)) RawVbeVideoModeInfo;

namespace vbe
{
    // VbeInfo <= RawVbeInfoBlock
    class VbeInfo
    {
    private:
        // Forbids copying.
        VbeInfo(const VbeInfo& other);
        const VbeInfo& operator = (const VbeInfo& other);
    public:
        VbeInfo(const RawVbeInfoBlock& raw);

        // All pointers below are kmalloc-ed, and need deleted
        ~VbeInfo();

        char* oemString;
        uint16_t* modeList;
        uint32_t numModes;
        uint32_t capabilityFlags;
        uint16_t totalMemory;                   // as # of 64KB blocks

        // Whether video card is compatible with vbe 2.0/3.0
        bool vbe2;
        bool vbe3;
    };

    // VideoModeInfo <= RawVbeVideoModeInfo
    class VideoModeInfo
    {
    public:
        VideoModeInfo(const RawVbeVideoModeInfo& raw);

        VideoModeInfo(const VideoModeInfo& other);
        VideoModeInfo& operator= (const VideoModeInfo& other);

        // All pointers below are kmalloc-ed, and need deleted
        ~VideoModeInfo();

        // The RGBMask looks like: "rrrrr___ggggggggbbbbb___"
        //      where '_' represents not present
        //      and if there is only 24 bits BPP, the last 8+1 chars are '\0'
        //      (BPP = bitsPerPixel)
        char RGBMask[33];

        uint16_t xRes, yRes;
        uint8_t  numPlanes;
        uint8_t  numImagePages;
        uint8_t  bitsPerPixel;                  // VBE calls this BBP, but it's also per color
        uint32_t physBase;                      // LFB (Linear Framebuffer) PHYSICAL addr
    };

    // A helper to abstract pixel encoding and memory accessing.
    // REQUIREMENT: bits per pixel per COLOR = 8
    // This structure has NO LOCK
    class VBEMemHelp
    {
    private:
        // Unit: bytes
        uint8_t pixelSize;

        // Unit: bytes
        size_t totalSize;
        const VideoModeInfo info;
        uint8_t* vmem;

        uint8_t colorCode(char colorCharName);
        uint8_t rCode, gCode, bCode, oCode;
        uint8_t maskCode[4];
    public:

        // vmem should be decided by Virtual Memory Manager
        VBEMemHelp(const VideoModeInfo& _info, uint8_t* vmem);
        ~VBEMemHelp();

        VBEMemHelp& put(size_t x, size_t y, uint8_t red, uint8_t green, uint8_t blue);
        VBEMemHelp& get(size_t x, size_t y, uint8_t *red, uint8_t *green, uint8_t *blue);
        VBEMemHelp& cls(uint8_t val);
        VBEMemHelp& copy(uint8_t* buildBuffer);
    };

    // WARNING: previously returned information is clobbered when calling any of
    //      these functions. Please make a backup if necessary.
    //          It does not help you back them up because most of the information
    //      is not very crucial and doing the backup might waste both time and memory.
    RawVbeInfoBlock* getVbeInfo();
    RawVbeVideoModeInfo* getVideoModeInfo(uint16_t mode);
    Maybe<VideoModeInfo> findVideoModeInfo(function<bool (VideoModeInfo)> fn);
}

#endif//_UI_VBE_H
