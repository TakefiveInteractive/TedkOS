#ifndef _ERR_HANDLER_H_
#define _ERR_HANDLER_H_


struct __attribute__ ((__packed__)) idt_stack_t {
    uint32_t EIP;
    uint16_t _padding_;
    uint16_t CS;
    uint32_t EFLAGS;
};

extern const unsigned long int ErrorCodeInExceptionBitField;

/*
 * This function is invoked by the common interrupt function when an 'error' occurs.
 */
extern "C" void exception_handler_with_number(size_t index, unsigned long int code, idt_stack_t *info);

template <uint32_t idx>
void trigger_exception()
{
    __asm__ __volatile__ (
        "int %0;\n"
        :
        : "i" (idx)
        : "cc");
}

#endif

