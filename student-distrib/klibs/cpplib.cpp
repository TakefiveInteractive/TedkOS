#include <inc/klibs/cpplib.h>

void __cxa_pure_virtual ()
{
    // Called when someone invokes a nonimplemented virtual function
    // Implemented with a CPU exception
    asm volatile("int $0x16;");
}

extern "C" int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle)
{
    // register a function to be called by exit or when a shared library is unloaded
    // we don't use shared libraries, so this is a dummy
    return 0;
}
