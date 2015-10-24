/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 expandtab
 */

#include <inc/i8259.h>
#include <inc/lib.h>
#include <inc/spinlock.h>
#include <inc/error.h>

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

#define SLAVE_PIN 0x02

/* Private functions */
static int setup_irq(unsigned int irq, unsigned int device_id,
        irq_good_handler_t handler, unsigned int policy_flags);
static void handle_level_irq(unsigned int irq, irq_desc_t* desc);
static void send_eoi_nolock(uint32_t irq_num);
static void disable_irq_nolock(uint32_t irq_num);
static void enable_irq_nolock(uint32_t irq_num);

// TODO: discuss this!
irq_desc_t irq_descs [NR_IRQS];

/* Initialize the 8259 PIC */
/* This MUST be called by kernel.c: entry(), BEFORE any other i8259's function is invoked!!! */
/*           (because this function is NOT protected by lock)                                */
void i8259_init(void)
{
    master_mask = 0xFF;
    slave_mask = 0xFF;

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

    // Initialize DS that stores IRQ state
    for(int i = 0; i < NR_IRQS; i++)
    {
        irq_descs[i].actionsLock  = SPINLOCK_UNLOCKED;
        irq_descs[i].lock  = SPINLOCK_UNLOCKED;
        irq_descs[i].depth = 0;
        init_list(&irq_descs[i].actions);
    }
}

/**
 * Enable (unmask) the specified IRQ
 * @param irq_num [interrupt pin number]
 */
void enable_irq(uint32_t irq_num)
{
    uint32_t flag;
    spin_lock_irqsave(&irq_descs[irq_num].lock, flag);
    enable_irq_nolock(irq_num);
    spin_unlock_irqrestore(&irq_descs[irq_num].lock, flag);
}

static void enable_irq_nolock(uint32_t irq_num)
{
    uint8_t irq;
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
}

/**
 * Disable (mask) the specified IRQ
 * @param irq_num [interrupt pin number]
 */
void disable_irq(uint32_t irq_num)
{
    uint32_t flag;
    spin_lock_irqsave(&irq_descs[irq_num].lock, flag);
    disable_irq_nolock(irq_num);
    spin_unlock_irqrestore(&irq_descs[irq_num].lock, flag);
}

static void disable_irq_nolock(uint32_t irq_num)
{
    uint8_t irq;
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
}

/**
 * Send end-of-interrupt signal for the specified IRQ
 * @param irq_num [interrupt pin number]
 */
void send_eoi(uint32_t irq_num)
{
    uint32_t flag;
    spin_lock_irqsave(&irq_descs[irq_num].lock, flag);
    send_eoi_nolock(irq_num);
    spin_unlock_irqrestore(&irq_descs[irq_num].lock, flag);
}


/**
 * Send end-of-interrupt signal for the specified IRQ
 * @param irq_num [interrupt pin number]
 *  This version will NOT lock the spinlock.
 */
static void send_eoi_nolock(uint32_t irq_num)
{
    if (irq_num < 8) {
        outb(PIC_EOI, MASTER_8259_PORT);
    } else {
        outb(PIC_EOI, MASTER_8259_PORT);
        outb(PIC_EOI, SLAVE_8259_PORT);
    }
}

// We use INTERRUPT Gate so interrupt Must have been disabled.
// REMEMBER to sti() on ALL exiting conditions (whether successful or not)
int irq_int_entry (int irq)
{
    if (irq >= NR_IRQS)
    {
        sti();
        return -EINVAL;
    }
    if (irq < 0)
    {
        sti();
        return -EINVAL;
    }

    irq_desc_t* desc = irq_descs + irq;
    handle_level_irq(irq, desc);

    return 1;
}

int bind_irq(unsigned int irq, unsigned int device_id,
        irq_good_handler_t handler, unsigned int policy_flags)
{
    if (irq >= NR_IRQS) return -EINVAL;
    if (irq < 0) return -EINVAL;
    if (!handler) return -EINVAL;
    return setup_irq(irq, device_id, handler, policy_flags);
}

void unbind_irq(unsigned int irq, unsigned int device_id)
{
    uint32_t flag;
    irq_desc_t* this_desc = irq_descs + irq;
    irqaction_list* list = &this_desc->actions;
    spin_lock_irqsave(&this_desc->actionsLock, flag);

    while(1)
    {
        irqaction* ptr;
        ptr = find_action(list, device_id, NULL);
        if (!ptr)
            break;
        remove_action(list, ptr);
    }
    if(!find_action(list, -1, NULL))
    {
        spin_unlock(&this_desc->actionsLock); sti();
        cli(); spin_lock(&this_desc->lock);
        disable_irq_nolock(irq);
        spin_unlock_irqrestore(&this_desc->lock, flag);
    }
    else spin_unlock_irqrestore(&this_desc->actionsLock, flag);
}

// Condition: interrupt must have already been disabled.
static void handle_level_irq(unsigned int irq, irq_desc_t* desc)
{
    irqaction* action;
    unsigned int flag;

    // Interrupt is still disabled before this line
    spin_lock(&desc->lock);

    // Validate IRQ state here.
    if (desc->depth + 1 >= MAX_DEPTH)
    {
        // !!! Strange: if no eoi is sent, the interrupt never happens again
        send_eoi_nolock(irq);
        spin_unlock(&desc->lock);
        sti();
        return;
    }
    desc->depth++;
    send_eoi_nolock(irq);


    // State is valid and executing other Interrupts is allowed now.
    spin_unlock(&desc->lock);
    sti();
    nop();

    // interrupt must be enabled before this line.
    spin_lock_irqsave(&desc->actionsLock, flag);
    for (action = first_action(&desc->actions); action; action = action->next)
        action->handler(irq, action->dev_id);
    spin_unlock_irqrestore(&desc->actionsLock, flag);

    spin_lock_irqsave(&desc->lock, flag);
    desc->depth--;
    spin_unlock_irqrestore(&desc->lock, flag);
}

static int setup_irq(unsigned int irq, unsigned int device_id,
        irq_good_handler_t handler, unsigned int policy_flags)
{
    int ret = 0;
    unsigned int flag;
    irq_desc_t* this_desc = irq_descs + irq;
    irqaction_list* list = &this_desc->actions;

    spin_lock_irqsave(&this_desc->actionsLock, flag);
    if(!first_action(list))
    {
        spin_unlock(&this_desc->actionsLock); sti();
        cli(); spin_lock(&this_desc->lock);
        enable_irq_nolock(irq);
        spin_unlock(&this_desc->lock); sti();
        cli(); spin_lock(&this_desc->actionsLock);
    }
    if(add_action(list, handler, policy_flags, 0, device_id) != 0)
        ret = -ENOMEM;
    spin_unlock_irqrestore(&this_desc->actionsLock, flag);

    return ret;
}
