#ifndef _UI_VBE_H
#define _UI_VBE_H

#include <inc/x86/real.h>
#include <inc/klibs/maybe.h>

typedef struct {
    char VbeSignature[4];                   // == "VESA"
    uint16_t VbeVersion;                    // == 0x0300 for VBE 3.0
    uint16_t OemStringPtr[2];               // isa vbeFarPtr
    uint8_t Capabilities[4];
    uint16_t VideoModePtr[2];               // isa vbeFarPtr
    uint16_t TotalMemory;                   // as # of 64KB blocks
} __attribute__((packed)) VbeInfoBlock;

extern Maybe<VbeInfoBlock> getVbeInfo();

#endif//_UI_VBE_H
