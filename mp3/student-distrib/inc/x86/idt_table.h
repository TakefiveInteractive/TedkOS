#ifndef _IDT_TABLE_H_
#define _IDT_TABLE_H_

typedef void (*vector_extracting_handler)(void);

extern vector_extracting_handler raw_interrupt_handlers[NUM_VEC];

extern void init_idt_table();
extern const unsigned long int ErrorCodeInExceptionBitField;

#endif
