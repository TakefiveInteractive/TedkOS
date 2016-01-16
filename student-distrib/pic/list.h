#ifndef _PIC_PRIV_LIST_H
#define _PIC_PRIV_LIST_H

#include <inc/i8259_extra.h>

#define _PIC_ACTION_LIST_SIZE   256

typedef struct {
    int size;           // How many data are actually stored
	irqaction_t data[_PIC_ACTION_LIST_SIZE];
    irqaction_t *head, *tail;
    // Note that if either of head, tail is NULL, both are NULL.
    //           if either of head, tail is NOT NULL, both are NOT NULL.
} irqaction_list;

/****
 *
 * WARNING!!!!
 *   This DS will NOT lock a spinlock !!!
 *
 ****/

/************ These DS functions can be tested *****************/

// Initialize an empty list.
extern void init_list(irqaction_list* list);

// Add a new action in the list, initilized with arguments passed in.
// This function returns 0 upon success.
// It might return nonzero due to capaticy issues.
extern int add_action(irqaction_list* list, irq_good_handler_t handler, unsigned int policy_flags, unsigned int mask, unsigned int dev_id);

// Returns the first action in list.
//  If there is NO ACTION in list, return NULL
extern irqaction* first_action(irqaction_list* list);

// Find action according to device id and handler
// Both should match.
// Return the INDEX in the array, or -1 if not found
// IF hander_to_find == NULL, then match only using deviceId_to_find
// IF device_id < 0, match only using handler.
// IF both NULL and dev < 0, then match everything.
extern irqaction* find_action(irqaction_list* list, unsigned int deviceId_to_find, irq_good_handler_t handler_to_find);

// Pass the index to remove as itemIdx
extern void remove_action(irqaction_list* list, irqaction* item);

#endif /*_PIC_PRIV_LIST_H*/
