/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include <inc/i8259.h>
#include <inc/lib.h>
#include <inc/spinlock.h>

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

#define SLAVE_PIN 0x02
spinlock_t lock = SPINLOCK_UNLOCKED;

/* Private functions */
static int setup_irq(unsigned int irq, unsigned int device_id,
        irq_good_handler_t handler, unsigned int policy_flags);
static void handle_level_irq(unsigned int irq, irq_desc_t* desc);
static int handle_irq_event(unsigned int irq, irqaction* action);

// TODO: discuss this!
irq_desc_t irq_descs [NR_IRQS];

/* Initialize the 8259 PIC */
void i8259_init(void)
{
    master_mask = 0xFF;
    slave_mask = 0xFF;

    uint32_t flag;
    spin_lock_irqsave(&lock, flag);

    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);

    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    outb(ICW4, MASTER_8259_PORT + 1);

    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW4, SLAVE_8259_PORT + 1);

    outb(master_mask, MASTER_8259_PORT + 1);
    outb(slave_mask, SLAVE_8259_PORT + 1);

    spin_unlock_irqrestore(&lock, flag);
}

/**
 * Enable (unmask) the specified IRQ
 * @param irq_num [interrupt pin number]
 */
void enable_irq(uint32_t irq_num)
{
    uint8_t irq;
    uint32_t flag;
    spin_lock_irqsave(&lock, flag);
    /* check interrupt number valid bounds */
    if (irq_num < 0 || irq_num > 15) return;
    /* master bounds */
    if (irq_num < 8) {
        irq = inb(MASTER_8259_PORT + 1) & ~(1 << irq_num);
        outb(irq, MASTER_8259_PORT + 1);
    /* slave irq */
    } else {
        irq = inb(SLAVE_8259_PORT + 1) & ~(1 << (irq_num - 8));
        outb(irq, SLAVE_8259_PORT + 1);
    }
    spin_unlock_irqrestore(&lock, flag);
}

/**
 * Disable (mask) the specified IRQ
 * @param irq_num [interrupt pin number]
 */
void disable_irq(uint32_t irq_num)
{
    uint8_t irq;
    uint32_t flag;
    spin_lock_irqsave(&lock, flag);
    /* check interrupt number valid bounds */
    if (irq_num < 0 || irq_num > 15) return;
    /* master bounds */
    if (irq_num < 8) {
        irq = inb(MASTER_8259_PORT + 1) | (1 << irq_num);
        outb(irq, MASTER_8259_PORT + 1);
    /* slave irq */
    } else {
        irq = inb(SLAVE_8259_PORT + 1) | (1 << (irq_num - 8));
        outb(irq, SLAVE_8259_PORT + 1);
    }
    spin_unlock_irqrestore(&lock, flag);
}

/**
 * Send end-of-interrupt signal for the specified IRQ
 * @param irq_num [interrupt pin number]
 */
void send_eoi(uint32_t irq_num)
{
    uint32_t flag;
    spin_lock_irqsave(&lock, flag);
    if (irq_num < 8) {
        outb(EOI | irq_num, MASTER_8259_PORT);
    } else {
        outb(EOI | SLAVE_PIN, MASTER_8259_PORT);
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
    }
    spin_unlock_irqrestore(&lock, flag);
}


int irq_int_entry (int irq, unsigned int dev_id)
{
    irq_desc_t* desc = irq_descs + irq;
    if (irq >= NR_IRQS) return -1;
    if (desc->depth + 1 >= MAX_DEPTH) return -1;
    desc->depth++;

    handle_level_irq(irq, desc);

    return 1;
}

int bind_irq(unsigned int irq, unsigned int device_id,
        irq_good_handler_t handler, unsigned int policy_flags)
{
    int retval;
    if (irq >= NR_IRQS) return -1;
    if (!handler) return -1;
    retval = setup_irq(irq, device_id, handler, policy_flags);

    return retval;
}

void unbind_irq(unsigned int irq, unsigned int device_id)
{
    irq_desc_t* this_desc = irq_descs + irq;
	uint32_t flag;
	irqaction_list* list = &this_desc->actions;
    spin_lock_irqsave(&this_desc->lock, flag);

    while(1)
    {
        int idx;
        idx = find_action(list, device_id, NULL);
        if (!idx)
            break;
        remove_action(list, idx);
    }
    if(!find_action(list, -1, NULL))
        disable_irq(irq);
    spin_unlock_irqrestore(&this_desc->lock, flag);
}

static void handle_level_irq(unsigned int irq, irq_desc_t* desc)
{
    irqaction* action;
    unsigned int flag;
    spin_lock_irqsave(&desc->lock, flag);
    send_eoi(irq);
    action = &desc->actions.data[desc->actions.firstDataIdx];
    spin_unlock_irqrestore(&desc->lock, flag);
    handle_irq_event(irq, action);
}

static int handle_irq_event(unsigned int irq, irqaction* action)
{
    int ret = 0;
    for (; action; action = action->next)
        ret = action->handler(irq, action->dev_id);
    return ret;
}

static int setup_irq(unsigned int irq, unsigned int device_id,
        irq_good_handler_t handler, unsigned int policy_flags)
{
    irq_desc_t* this_desc = irq_descs + irq;
	irqaction_list* list = &this_desc->actions;
    int ret;
    unsigned int flag;

    spin_lock_irqsave(&this_desc->lock, flag);
    if(!find_action(list, device_id, handler))
        enable_irq(irq);
    //WANRNING!!!: ret should be general kernel error instead of linked list's private ret
    ret = add_action(list, handler, policy_flags, 0, device_id);
    spin_unlock_irqrestore(&this_desc->lock, flag);

    return ret;
}
