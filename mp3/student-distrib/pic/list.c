#include <stddef.h>
#include "list.h"
#include <inc/klibs/lib.h>

// firstDataIdx == -1 <=> List is EMPTY

// Actual Insertion/Removal/Access is Array-based.
// (that is, items always occupy the first few cells in array)
// (And when removing an item, we copy everything after it forward)

void init_list(irqaction_list* list)
{
    list->size = 0;
    list->head = list->tail = NULL;
}

int add_action(irqaction_list* list, irq_good_handler_t handler, unsigned int policy_flags, unsigned int mask, unsigned int dev_id)
{
    irqaction* added;
    if(list->size == _PIC_ACTION_LIST_SIZE)
        return -1;
    added = list->data + list->size;
    added->handler = handler;
    added->policy_flags = policy_flags;
    added->mask = mask;
    added->dev_id = dev_id;
    added->next = NULL;
    if(list->tail)
    {
        irqaction* oldtail = list->tail;
        oldtail->next = added;
        added->prev = oldtail;
        list->tail = added;
    }
    else    // The list was originally empty
    {
        added->prev = NULL;
        list->head = list->tail = added;
    }
    list->size++;
    return 0;
}

irqaction* first_action(irqaction_list* list)
{
    return list->head;
}

irqaction* find_action(irqaction_list* list, int deviceId_to_find, irq_good_handler_t handler_to_find)
{
    int i;
    const int len = list->size;
    for(i = 0; i < len; i++)
        if((deviceId_to_find < 0 || list->data[i].dev_id == deviceId_to_find)
         &&(handler_to_find == NULL || list->data[i].handler == handler_to_find))
            return list->data + i;
    return NULL;
}

void remove_action(irqaction_list* list, irqaction* item)
{
    irqaction *origPrev, *origNext, *clobber, *newHead, *newTail;
    if(list->size == 0 || (item - list->data) >= list->size)
        return;

    if(list->size == 1)
    {
        list->size = 0;
        list->head = list->tail = NULL;
        return;
    }

    // Firstly: calculate the new head and tail of the list.
    newHead = list->head;
    if(item == list->head)
        newHead = newHead->next;

    newTail = list->tail;
    if(item == list->tail)
        newTail = newTail->prev;

    // secondly: detach the item node from the linked list.
    // origPrev --next--> item <--prev-- origNext
    origPrev = item->prev;
    origNext = item->next;
    if(origPrev)
        origPrev->next = origNext;
    if(origNext)
        origNext->prev = origPrev;
    item->prev = NULL;
    item->next = NULL;

    // thirdly, reset the next and prev pointers of origNext and origPrev
    //  WARNING! be careful to adjust the pointers related to moved nodes.
    for(clobber = item; clobber < list->tail; clobber++)
    {
        irqaction *srcPrev, *srcNext, *src;
        src = clobber + 1;
        memcpy(clobber, src, sizeof(irqaction));

        // These two nodes' attributes are affected and should be adjusted.
        srcPrev = src->prev;
        srcNext = src->next;

        // adjust the pointers after moving the node to different addr. in mem.
        if(srcPrev)
            srcPrev->next = clobber;
        if(srcNext)
            srcNext->prev = clobber;
    }

    list->size--;
    list->head = newHead;
    list->tail = newTail;
}

