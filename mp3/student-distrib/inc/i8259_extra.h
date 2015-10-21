#ifndef _I8259_EXTRA_H_
#define _I8259_EXTRA_H_

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
typedef irqaction irqaction_t;

#endif

