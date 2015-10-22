#ifndef _IDT_TABLE_H_
#define _IDT_TABLE_H_

extern void** raw_interrupt_handlers;
extern void interrupt_handler_with_number (size_t index, uint32_t code);

#endif
