#include <stddef.h>
#include <stdint.h>
#include <inc/x86/err_handler.h>
#include <inc/lib.h>

const static size_t err_code_offset = (8 + 3) * sizeof(uint32_t);

void __attribute__((fastcall)) exception_handler(size_t vec, uint32_t code)
{
    int have_error_code = ErrorCodeInExceptionBitField & (1 << vec);
    printf("WTF Exception occured!\n");
    if (have_error_code)
    {
        printf("Error code is %x", code);
    }
}

