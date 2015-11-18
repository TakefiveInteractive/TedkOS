#include "draw_nikita.h"
#include <stdint.h>
#include <stddef.h>
#include <inc/fs/kiss_wrapper.h>
#include <inc/ui/vbe.h>
#include <inc/x86/real.h>
#include <inc/klibs/palloc.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/x86/err_handler.h>

using namespace vbe;
using namespace palloc;

void paint_screen(uint8_t *pixel, uint8_t *source)
{
    for (size_t x = 0; x < 1024 * 768; x++)
    {
        pixel[2] = source[0];
        pixel[1] = source[1];
        pixel[0] = source[2];
        pixel += 3;
        source += 4;
    }
}

void draw_nikita()
{
    int32_t keyb = fs_open("/dev/keyb");
    int32_t rtc =fs_open("/dev/rtc");
    char buf[32];

    printf("keyboard opened\n");

    cli();

    auto vbeInfoMaybe = getVbeInfo();
    if(vbeInfoMaybe)
    {
        VbeInfo vbeInfo (+vbeInfoMaybe);
        printf("VBE Information:\n");
        if(vbeInfo.vbe2)
            printf("\tSupports VBE 2.0\n");
        if(vbeInfo.vbe3)
            printf("\tSupports VBE 3.0\n");
        printf("\tVbeCapability = %x\n", vbeInfo.capabilityFlags);
        printf("\tOEM = %s\n", vbeInfo.oemString);
        printf("\tTotal Memory = %d KB\n", ((uint32_t)(vbeInfo.totalMemory)) * 64);

        printf("\tAvailable Video Modes:\n");
        for(int i=0; vbeInfo.modeList[i]!=0xffff; i++)
        {
            uint16_t mode = vbeInfo.modeList[i];
            printf("\t Mode %x:  ", mode);

            // Query more about this mode
            auto modeInfoMaybe = getVideoModeInfo(mode);
            if(modeInfoMaybe)
            {
                VideoModeInfo modeInfo(+modeInfoMaybe);
                printf(" XRes=%d, YRes=%d, BaseAddr=%x, Color=%s\n",
                    modeInfo.xRes,
                    modeInfo.yRes,
                    modeInfo.physBase,
                    modeInfo.RGBMask);
            }
            else printf("\t\tCan't get any information\n");
        }
        printf("\n");
    }


    //------------- Try to draw 1024 * 768 HD Graphics ----------------
    auto Mode118Maybe = getVideoModeInfo(0x118);
    if(Mode118Maybe) ;
    else printf("1024*768 24bits mode is NOT supported.\n");
    VideoModeInfo Mode118(+Mode118Maybe);
    uint32_t Mode118Mem = Mode118.physBase;

    uint16_t orig_mode;

    // Back up current mode.
    real_context_t real_context;
    real_context.ax = 0x0f00;
    legacyInt(0x10, real_context);
    orig_mode = real_context.ax & 0x00ff;

    LOAD_4MB_PAGE(Mode118Mem>>22, Mode118Mem, PG_WRITABLE);
    RELOAD_CR3();

    auto physAddr = physPages.allocPage(true);
    if (!physAddr) trigger_exception<27>();

    LOAD_4MB_PAGE(+physAddr, +physAddr << 22, PG_WRITABLE);
    RELOAD_CR3();
    uint8_t *nikita = (uint8_t *)((uint32_t)(+physAddr) << 22);

    dentry_t dentry;
    filesystem::read_dentry((uint8_t*)"landscape", &dentry);
    filesystem::read_data(dentry.inode, 0, nikita, 1024 * 768 * 4);

    uint8_t *pixel = (uint8_t *) Mode118Mem;

    paint_screen(pixel, nikita);

    // Change to HD Video Mode
    real_context.ax=0x4F02;
    real_context.bx=0x8118;
    legacyInt(0x10, real_context);

    // Wait for user
    sti();
    fs_read(keyb, buf, 1);
    cli();

    // Change back to original mode
    real_context.ax=orig_mode;
    legacyInt(0x10, real_context);

    printf("Back to KERNEL!\n");

    sti();

    fs_close(keyb);
    fs_close(rtc);
}
