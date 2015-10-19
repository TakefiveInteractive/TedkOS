#include "err_handler.h"

void exception_handler(void)
{

}

__asm__("_raw_exception_handler:    \n\
        pushad          \n\
        cld             \n\
        call exception_handler      \n\
        popad           \n\
        iret            \n\
");

