#include <stddef.h>
#include <stdint.h>
#include <inc/x86/err_handler.h>

const static size_t err_code_offset = (8 + 3) * sizeof(uint32_t);

void exception_handler(size_t vec)
{

}

