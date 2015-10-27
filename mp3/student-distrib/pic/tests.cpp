#include <iostream>
#include <vector>

using namespace std;

#include <cstdlib>
#include <limits.h>
#include <gtest/gtest.h>

#include <inc/i8259_extra.h>

#define _PIC_ACTION_LIST_SIZE   256

typedef struct {
    int size;           // How many data are actually stored
	irqaction_t data[_PIC_ACTION_LIST_SIZE];
    irqaction_t *head, *tail;
    // Note that if either of head, tail is NULL, both are NULL. 
    //           if either of head, tail is NOT NULL, both are NOT NULL. 
} irqaction_list;

extern "C" void init_list(irqaction_list* list);

extern "C" int add_action(irqaction_list* list, irq_good_handler_t handler, unsigned int policy_flags, unsigned int mask, unsigned int dev_id);

extern "C" irqaction* first_action(irqaction_list* list);

extern "C" irqaction* find_action(irqaction_list* list, int deviceId_to_find, irq_good_handler_t handler_to_find);

extern "C" void remove_action(irqaction_list* list, irqaction* item);

TEST(NextPrevPointers, SimpleAdds) {
	irqaction_list list;
	init_list(&list);
	vector<int> soln;
	srand(0);
	for(int i=0; i<_PIC_ACTION_LIST_SIZE; i++)
	{
		int val=rand();
		soln.push_back(val);
		add_action(&list, NULL, 0, 0, val);
	}
	int compi = 0;
	for(irqaction* curr = first_action(&list); curr; curr=curr->next)
	{
		ASSERT_TRUE(curr->dev_id == soln[compi]);
		if(curr->next)
			ASSERT_TRUE(curr == curr->next->prev);
		else ASSERT_TRUE(curr == list.tail);
		if(curr->prev)
			ASSERT_TRUE(curr == curr->prev->next);
		else ASSERT_TRUE(curr == list.head);
		compi++;
	}
}

TEST(NextPrevPointers, HarshRemovals) {
	irqaction_list list;
	init_list(&list);
	vector<int> soln;
	srand(0);
	for(int i=0; i<_PIC_ACTION_LIST_SIZE; i++)
	{
		int val=rand();
		soln.push_back(val);
		add_action(&list, NULL, 0, 0, val);
	}
	int compi = 0;
	for(irqaction* curr = first_action(&list); curr; curr=curr->next)
	{
		ASSERT_TRUE(curr->dev_id == soln[compi]);
		if(curr->next)
			ASSERT_TRUE(curr == curr->next->prev);
		else ASSERT_TRUE(curr == list.tail);
		if(curr->prev)
			ASSERT_TRUE(curr == curr->prev->next);
		else ASSERT_TRUE(curr == list.head);
		compi++;
	}
}

TEST(Empty, FirstActionIsNULL) {
	irqaction_list list;
	init_list(&list);
	ASSERT_TRUE(first_action(&list) == NULL);
}

TEST(Memory, AddMoreThanLISTSIZE) {
	irqaction_list list;
	init_list(&list);
	for(int i=0; i<_PIC_ACTION_LIST_SIZE; i++)
		ASSERT_TRUE(add_action(&list, 0, 0, 0, 0) == 0);
	ASSERT_TRUE(add_action(&list, 0,0,0,0) != 0);
}

TEST(Sustainability, NeverShrink) {
	irqaction_list list;
	init_list(&list);
	for(int i=0; i<5; i++)
	{
		for(int j=0; j<2 * _PIC_ACTION_LIST_SIZE; j++)
			add_action(&list, 0, 0, 0, 0);
		for(int j=0; j<_PIC_ACTION_LIST_SIZE - 1; j++)
		{
			irqaction* p = find_action(&list,-1,NULL);
			ASSERT_TRUE(p!=NULL);
			remove_action(&list, p);
		}
	}
	for(int j=0; j<2 * _PIC_ACTION_LIST_SIZE; j++)
		add_action(&list, 0, 0, 0, 0);
	for(int j=0; j<_PIC_ACTION_LIST_SIZE; j++)
	{
		irqaction* p = find_action(&list,-1,NULL);
		ASSERT_TRUE(p!=NULL);
		remove_action(&list, p);
	}
	for(int i=0; i<_PIC_ACTION_LIST_SIZE; i++)
		ASSERT_TRUE(add_action(&list, 0, 0, 0, 0) == 0);
	ASSERT_TRUE(add_action(&list, 0,0,0,0) != 0);
}
