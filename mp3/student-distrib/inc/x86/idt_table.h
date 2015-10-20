#ifndef _IDT_TABLE_H_
#define _IDT_TABLE_H_

typedef void (*vector_extracting_handler)(void) __attribute__((fastcall));
#ifdef __cplusplus
extern "C" {
#endif
    extern static const unsigned long int ErrorCodeInExceptionBitField;
    extern vector_extracting_handler interrupt_table[255];
#ifdef __cplusplus
}
#endif

#endif
