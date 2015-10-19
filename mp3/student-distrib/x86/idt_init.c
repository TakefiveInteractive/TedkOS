#include <inc/x86/idt_init.h>

// No head and end commas
#define GENERATE_RAW_HANDLER(ID) \
	

const uint32_t idtRawHandlers[256] = {
	ARR_REPEAT_32(raw_exception_handler),
	ARR_REPEAT_16(raw_pic_irq_handler),
	ARR_REPEAT_80(raw_nothing_handler),
	raw_syscall_handler,

}

void init_idt(void)
{
}
