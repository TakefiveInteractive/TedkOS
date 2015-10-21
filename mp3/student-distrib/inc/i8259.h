/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include <inc/spinlock.h>
#include <stddef.h>
#include <stdint.h>

/* Ports that each PIC sits on */
#define MASTER_8259_PORT 0x20
#define SLAVE_8259_PORT  0xA0

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1    0x11
#define ICW2_MASTER   0x20
#define ICW2_SLAVE    0x28
#define ICW3_MASTER   0x04
#define ICW3_SLAVE    0x02
#define ICW4          0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI             0x60
#define NR_IRQS         16
#define MAX_DEPTH       8

// Return value is preserved.
// Currently all functions return 0.
// Later it might be used to indicate EVENT_NOT_HANDLED
typedef int (*irq_good_handler_t)(int irq, unsigned int dev_id);

typedef struct irqaction_t {
    irq_good_handler_t handler;
    unsigned int policy_flags;
    unsigned int mask;
    unsigned int dev_id;     /* A interger to differentiate different devices */
    struct irqaction_t *next; /* not used for now */
} irqaction;

// pic/list.h must be included after irqaction type has been declared.
#include <pic/list.h>

typedef struct {
    unsigned int status;            /* IRQ status */
    unsigned int depth;             /* nested irq disables */
	irqaction_list actions;
    spinlock_t lock;
} irq_desc_t;
extern irq_desc_t irq_descs [NR_IRQS];

/************* PIC functions *****************/

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

/*
 * int irq_int_entry (int irq, pt_reg* saved_reg)
 * Description: irq_interrupt_entry
 *     equivalent to do_IRQ in linux.
 *     This function is called by IDT, for every IRQ from 0 to 15.
 * Input:
 *     irq -- The currently called IRQ number, because this function is called
 *     by multiple IRQ nums, this variable used to differentiate between them.
 *     saved_reg -- You should not change it or read it.
 * Return value: used to indicate success or not.
 *    WARNING: return value is currently not used.
 */
int irq_int_entry (int irq, unsigned int dev_id);

 /*
  * bind_irq(unsigned int irq, irq_good_handler_t driver_level_handler, unsigned int policy_flags)
  * Description:
  *     This is equivalent to linux's request_irq() Add driver_level_handler
  *     to the handler linked list, and set policy_flags (if necessary)
  * Return val: SUCCESS = 0. Otherwise non zero.
  * WARNING: THIS RET VAL MUST BE SET
  */
int bind_irq(unsigned int irq, unsigned int device_id,
    irq_good_handler_t driver_level_handler, unsigned int policy_flags);

// Similar to free_irq in linux. Unbinds handler of device_id from irq
void unbind_irq(unsigned int irq, unsigned int device_id);

#endif /* _I8259_H */
