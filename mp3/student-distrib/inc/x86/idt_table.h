#ifndef _IDT_TABLE_H_
#define _IDT_TABLE_H_

typedef void (*vector_extracting_handler)(void);
#ifdef __cplusplus
extern "C" {
#endif
    extern vector_extracting_handler raw_interrupt_handlers[NUM_VEC];
#ifdef __cplusplus
}
#endif

#endif
