#include "draw_nikita.h"
#include <stdint.h>
#include <stddef.h>
#include <inc/ui/compositor.h>
#include <inc/syscalls/filesystem_wrapper.h>

using ui::Compositor;

void draw_nikita()
{
    int32_t keyb = syscall::fops::open("/dev/keyb");
    char buf[32];

    printf("keyboard opened\n");

    Compositor comp;
    comp.enterVideoMode();
    comp.drawNikita();

    // Wait for user
    syscall::fops::read(keyb, buf, 1);
    comp.enterTextMode();

    printf("Back to KERNEL!\n");

    syscall::fops::close(keyb);
}
