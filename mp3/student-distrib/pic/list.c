#include <stddef.h>
#include "list.h"

extern int add_action(irqaction_list* list, irq_good_handler_t handler, unsigned int policy_flags, unsigned int mask, unsigned int dev_id)
{
    return 0;
}

extern irqaction* first_action(irqaction_list* list)
{
    return NULL;
}

extern int find_action(irqaction_list* list, int deviceId_to_find, irq_good_handler_t handler_to_find)
{
    return 0;
}

extern void remove_action(irqaction_list* list, int itemIdx)
{

}

