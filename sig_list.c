#include <stdio.h>
#include <stdlib.h>

#include "sig_list.h"


bool is_null(ListNode * head)
{
	return head ->next == head;
}

void out(ListNode *head, unsigned int *number)
{
	if (is_null(head))
	{
		*number = 0;
		return;
	}
	*number = head ->next ->keyVal;

	ListNode *tmp = head ->next;
	head ->next = head ->next ->next;
	free(tmp);
}


void in(ListNode *head, unsigned int number)
{
	
	//新建一个链表
	ListNode *tmp = (ListNode *)malloc(sizeof(ListNode));
	if (tmp == NULL)
	{
		printf("malloc fail\n");
		return;
	}

	//尾插法，在链表的的尾部插入tmp，tmp先连上head
	tmp ->keyVal = number;
	tmp ->next = head;

	//遍历链表
	ListNode *p = head;
	while (p ->next != head)
	{
		p = p ->next;
	}
	//tmp的上一个连接上tmp，形成一个完整的链表
	p ->next = tmp;
}

void output(ListNode * head)
{
	ListNode *end = head;

	ListNode *tmp = head ->next;
	while (end != head ->next)
	{
		while ( tmp ->next != end ) 
		{
			tmp = tmp ->next;
		}
		printf ("%d ", tmp ->keyVal);
		end = tmp;
	}
}

//单向循环链表并初始化头结点
ListNode * init_head()
{
	ListNode *head = (ListNode *)malloc(sizeof(ListNode));
	if (head != NULL)
	{
		//初始化头节点里的keyVal
		head ->keyVal = 0;
		//申明是单向循环链表
		head ->next = head;
		return head;
	}
	return NULL;
}

// int main(int argc, char const *argv[])
// {
// 	//建立一个单向循环链表
// 	ListNode *head = init_head();
// 	for (int i = 0; i < 5; ++i)		//此处更改输入数量
// 	{
// 		add_tail(head);
// 	}
// 	output(head);
// 	return 0;
// }