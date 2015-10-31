/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 expandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include <stddef.h>
#include <stdint.h>
#include <inc/klibs/spinlock.h>
#include <inc/i8259_extra.h>

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
#define PIC_SPEC_EOI    0x20
#define NR_IRQS         16
#define MAX_DEPTH       8

// pic/list.h must be included after irqaction type has been declared.
#include <pic/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int status;            /* IRQ status, currently NOT used */
    unsigned int depth;             /* nested irq disables */
    irqaction_list actions;
    // If both of the locks must be locked, FIRST lock_IRQSAVE 'lock' BEFORE locking 'actionsLock'!
    spinlock_t lock;                /* lock for the eoi signal, and anything other than actions list */
    spinlock_t actionsLock;         /* lock for the actions list */
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
 * Return value: used to indicate success or not.
 *    WARNING: return value is currently not used.
 */
int irq_int_entry (int irq);

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

#ifdef __cplusplus
}
#endif

#endif /* _I8259_H */
