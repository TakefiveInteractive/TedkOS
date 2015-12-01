#include "draw_nikita.h"
#include <stdint.h>
#include <stddef.h>
#include <inc/ui/compositor.h>
#include <inc/syscalls/filesystem_wrapper.h>
#include <inc/x86/err_handler.h>
#include <inc/ece391syscall.h>

using ui::Compositor;


void draw_nikita()
{
    // Must use actual syscall if it's SUPPOSED to block!
    int32_t keyb = 0;

    char buf[32];

    // Wait for user
    ece391_read(keyb, buf, 1);

    Compositor *comp = Compositor::getInstance();
    comp->enterVideoMode();
    comp->drawNikita();

    // Wait for user
    ece391_read(keyb, buf, 1);
    comp->enterTextMode();

    printf("Back to KERNEL!\n");
}
