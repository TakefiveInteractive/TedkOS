#ifndef _PIC_PRIV_LIST_H
#define _PIC_PRIV_LIST_H

#include <inc/i8259.h>

#define _PIC_ACTION_LIST_SIZE   256

typedef struct {
	int firstDataIdx;
	irqaction_t data[_PIC_ACTION_LIST_SIZE];
	char hasDataHere[_PIC_ACTION_LIST_SIZE];
} irqaction_list;

/************ These DS functions can be tested *****************/

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
extern int find_action(irqaction_list* list, int deviceId_to_find, irq_good_handler_t handler_to_find);

// Pass the index to remove as itemIdx
static void remove_action(irqaction_list* list, int itemIdx);

#endif /*_PIC_PRIV_LIST_H*/
