#include <inc/klibs/panic.h>
#include <inc/x86/err_handler.h>
#include <inc/klibs/lib.h>

void kernelPanic(const char *message)
{
    printf("KERNEL PANIC: %s\n", message);
    trigger_exception<30>();
}
