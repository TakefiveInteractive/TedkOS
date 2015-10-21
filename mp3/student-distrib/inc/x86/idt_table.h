#ifndef _IDT_TABLE_H_
#define _IDT_TABLE_H_

typedef void __attribute__((fastcall)) (*vector_extracting_handler)(void);

#ifndef __cplusplus
    extern vector_extracting_handler *raw_interrupt_handlers;
#endif

#ifdef __cplusplus
extern "C" {
#endif
    extern const unsigned long int ErrorCodeInExceptionBitField;
#ifdef __cplusplus
}
#endif

#endif
