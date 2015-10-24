#ifndef _IDT_TABLE_H_
#define _IDT_TABLE_H_

typedef void (*vector_extracting_handler)(void);

#ifndef __cplusplus
    extern vector_extracting_handler raw_interrupt_handlers[NUM_VEC];
#endif

#ifdef __cplusplus
extern "C" {
#endif
    extern void init_idt_table();
    extern const unsigned long int ErrorCodeInExceptionBitField;
#ifdef __cplusplus
}
#endif

#endif
