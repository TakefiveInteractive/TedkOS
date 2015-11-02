#include <inc/klibs/cpplib.h>

void __cxa_pure_virtual ()
{
    // Called when someone invokes a nonimplemented virtual function
    // Implemented with a CPU exception
    asm volatile("int $0x16;");
}
