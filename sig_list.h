#ifndef SIG_LIST
#define SIG_LIST


#include <stdbool.h>

//单链表结构体
typedef struct struct_list_node
{
	unsigned int keyVal;
	struct struct_list_node *next;
}ListNode;



void in(ListNode* head, unsigned int number);

void out(ListNode* head, unsigned int *number);

bool is_null(ListNode * head);

ListNode * init_head();
void dele_list(ListNode *head);


#endif