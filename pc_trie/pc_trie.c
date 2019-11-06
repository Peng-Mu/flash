#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "pc_trie.h"

//typedef struct s_PcNode
//{
//	int len;//value to indentify if the node is inherited
//	int id;
//} t_PcNode;
//
//typedef struct s_PcSlot
//{
//	t_PcNode pc_node[4];
//	struct s_PcSlot *next;
//} t_PcSlot;

char* Bitmap_Create(int prefix_len)
{
	int i;
	int total = 1 << (prefix_len-1);	
	char *bitmap = (char*) malloc(total*sizeof(char)); 
	if (bitmap == NULL)
	{
		printf("malloc is error!");
		exit(1);
	}
	for (i = 1; i < total; i++)
		bitmap[i] = 0;
	return bitmap;
}

void Bitmap_Destroy(char *bitmap)
{
	free(bitmap);
}

t_PcNode* PcNode_Create(int num)
{
	int i;
	t_PcNode *node = (t_PcNode*) malloc(num*sizeof(*node));
	if (node == NULL) { printf("malloc error!\n"); exit(1); }
	memset(node, 0 , num*sizeof(*node));
	for (i = 0; i < num; i++)
	{
		node[i].id = -1;
		node[i].len = -1;
	}
	return node;
}

void PcNode_Destroy(t_PcNode *node_head)
{
	free(node_head);
}

t_PcSlot** Slot_Create(void)
{
	t_PcSlot **head = (t_PcSlot**) malloc(sizeof(*head));
	*head = NULL;
	return head;
}

t_PcSlot* Slot_Add(t_PcSlot **slot_head, int slot_num)
{
	int i;
	t_PcSlot **prev = slot_head;
	t_PcSlot *newslot = (t_PcSlot*) malloc(sizeof(*newslot));
	if (newslot == NULL) { printf("malloc is error!\n"); exit(1);}
	memset(newslot, 0, sizeof(*newslot));
	while(slot_num-- > 1 && *prev != NULL)
	{
		if (*prev == NULL) { printf("Error, the linklist is shorter than input num\n"); exit(1); }
		prev = &(*prev)->next;
	}
	newslot->next = *prev;
	*prev = newslot;
	for (i = 0; i < 4; i++)
	{
		newslot->pc_node[i].id = -1;
		newslot->pc_node[i].len = -1;
	}
	return newslot;
}

void Slot_Delete(t_PcSlot **slot_head, int slot_num)
{
	t_PcSlot **prev = slot_head;
	t_PcSlot *temp;
	while (slot_num-- > 1 && *prev != NULL)
	{
	//	if (*prev == NULL) { printf("No element in the linklist can be deleted!\n"); exit(1); }
		prev = &(*prev)->next;
	}
	if (*prev == NULL) { printf("No element in the linklist can be deleted!\n"); exit(1); }
	temp = *prev;
	*prev = temp->next;
	free(temp);
}

void Slot_Destroy(t_PcSlot **slot_head)
{
	t_PcSlot *temp, *p = *slot_head;
	
	while(p != NULL)
	{
		temp = p;
		p = p->next;
		free(temp);
	}
	free(slot_head);
}

t_PcSlot* Slot_Locate(t_PcSlot **slot_head, int slot_num)
{
	t_PcSlot *curr = *slot_head;
	while (slot_num-- > 1 && curr != NULL)
	{
	//	if (curr == NULL) { printf("The num is over the length of linklist!\n"); exit(1); }
		curr = curr->next;
	}
	if (curr == NULL) { printf("The num is over the length of linklist!\n"); exit(1); }
	return curr;
}

//t_PcSlot* Left_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num)
//{
//	int one_cnt;
//	t_PcSlot *slot;
//	if (bitmap[bitmap_num*2] == 0)
//		return NULL;
//	one_cnt = Count_One(bitmap, 1, bitmap_num*2);
//	slot = Slot_Locate(slot_head, one_cnt);
//	return slot;
//}
//
//t_PcSlot* Right_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num)
//{
//	int one_cnt;
//	t_PcSlot *slot;
//	if (bitmap[bitmap_num*2+1] == 0)
//		return NULL;
//	one_cnt = Count_One(bitmap, 1, bitmap_num*2+1);
//	slot = Slot_Locate(slot_head, one_cnt);
//	return slot;
//}
//
//t_PcSlot* Brother_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num)
//{
//	int one_cnt;
//	int brother_bitmap_num;
//	t_PcSlot *slot;
//	brother_bitmap_num = ((bitmap_num%2 == 0) ? bitmap_num+1 : bitmap_num-1);
//	if (bitmap[brother_bit_map_num] == 0)
//		return NULL;
//	one_cnt = Count_One(bitmap, 1, brother_bitmap_num);
//	slot = Slot_Locate(slot_head, one_cnt);
//	return slot;
//}
//
//t_PcSlot* Father_Slot(char *bitmap, t_PcSlot **slot_head, int bitmap_num)
//{
//	int one_cnt;
//	t_PcSlot *slot;
//	if (bitmap_num < 2)
//		return NULL;
//	one_cnt = Count_One(bitmap, 1, bitmap_num/2);
//	slot = Slot_Locate(slot_head, one_cnt);
//	return slot;
//}



int Count_One(char *bitmap, int start, int end)
{
	int bit_cnt = start;
	int one_cnt = 0;
	while (bit_cnt <= end)
	{
		if(bitmap[bit_cnt] == 1) one_cnt++;
		bit_cnt++;
	}
	return one_cnt;
}


t_PcNode* PcNode_Locate(char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head, int num)
{
	int bitmap_num, slot_num, slot_node_order;
	t_PcSlot *slot;
	if (num < 4)
		return &node_head[num-1];
	else
	{
		bitmap_num = num/4;
		slot_node_order = num % 4;
		if (bitmap[bitmap_num] == 0)
			return NULL;
		slot_num = Count_One(bitmap, 1, bitmap_num);
	//	if (one_cnt != NULL) 
	//		*one_cnt = slot_location;
		slot = Slot_Locate(slot_head, slot_num);
		return &slot->pc_node[slot_node_order];
	}
}

t_PcNode* Left_Child(char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head, int num, int pre_one_cnt, int *curr_one_cnt)
{
	int leftchild_num;
	t_PcNode *leftchild;
	t_PcSlot *slot;
	leftchild_num = 2*num;
	if (leftchild_num < 4)
		return &node_head[leftchild_num-1];
	*curr_one_cnt =  pre_one_cnt + Count_One(bitmap, num/4 + 1, leftchild_num/4);
	if (bitmap[leftchild_num/4] == 0)
		return NULL;
	slot = Slot_Locate(slot_head, *curr_one_cnt);
	leftchild = &slot->pc_node[leftchild_num % 4];
	return leftchild;
}

t_PcNode* Right_Child(char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head, int num, int pre_one_cnt, int *curr_one_cnt)
{
	t_PcNode *leftchild = Left_Child(bitmap, slot_head, node_head, num, pre_one_cnt, curr_one_cnt);
	if (leftchild == NULL)
		return NULL;
	return leftchild+1;
}


int PcTrie_Pfx_Num(char *data)
{
	int i = 0;
	int num = 1;
	t_PcNode *node;
	while (data[i] != '\0')
	{
		if (data[i] == '0') num = num*2;
		else if (data[i] == '1') num = num*2 + 1;
		i++;
	}
	return num;
}


void PcTrie_Pfx_Add(int num, int id, int length, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head)
{
	t_PcNode *node;
	t_PcSlot *slot;
	int one_cnt;
	if (num > 3 && bitmap[num/4] == 0)
	{
		bitmap[num/4] = 1;
		one_cnt = Count_One(bitmap, 1, num/4);
		slot = Slot_Add(slot_head, one_cnt);
		node = &slot->pc_node[num%4];
	}
	else
		node = PcNode_Locate(bitmap, slot_head, node_head, num);
	node->id = id;
	node->len = length;
	return;
}

void PcTrie_Pfx_Deliver(int num, int id, int length, int one_cnt, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head)
{
	int left_one_cnt = 0; 
	int right_one_cnt = 0;
	t_PcNode *leftchild, *rightchild;
	//the number 10 is prefix total length 9 add 1 in pc_trie
	if (num > (1<<10-1))
		return;
	leftchild = Left_Child(bitmap, slot_head, node_head, num, one_cnt, &left_one_cnt);
	rightchild = Right_Child(bitmap, slot_head, node_head, num, one_cnt, &right_one_cnt);
	if (leftchild == NULL && rightchild == NULL)
	{
		PcTrie_Pfx_Deliver(num*2, id, length, left_one_cnt, bitmap, slot_head, node_head); 		
		PcTrie_Pfx_Deliver(num*2+1, id, length, right_one_cnt, bitmap, slot_head, node_head); 	
		return;
	}
	if (leftchild->len > length && rightchild->len > length)
		return;

	if (leftchild->id > 0 && leftchild->len < length)
	{
		leftchild->id = id;
		leftchild->len = length;
		PcTrie_Pfx_Deliver(num*2, id, length, left_one_cnt, bitmap, slot_head, node_head); 		
	}
	if (rightchild->id > 0 && rightchild->len < length)
	{
		rightchild->id = id;
		rightchild->len = length;
		PcTrie_Pfx_Deliver(num*2+1, id, length, right_one_cnt, bitmap, slot_head, node_head);
	}
	return;
}

void PcTrie_Pfx_Inherit(int num, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head)
{
	t_PcNode *node, *inherit_node;
	int inherit_num; 
		
	node = PcNode_Locate(bitmap, slot_head, node_head, num);
	inherit_num = num/2;
	while (inherit_num/4 > 0 && bitmap[inherit_num/4] != 1)
	{
		inherit_num /= 2;
	}
	if (inherit_num/4 == 0 && node_head[inherit_num-1].id < 0)
	{
		inherit_node = &node_head[0];	
	}
	else
		inherit_node = PcNode_Locate(bitmap, slot_head, node_head, inherit_num);
	node->id = inherit_node->id;
	node->len = inherit_node->len;
}

void PcTrie_Add_Update(char *data, int id, int length, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head)
{
	int brotherslot_bit, one_cnt, num, i;
	t_PcSlot *leftslot, *rightslot;
	num = PcTrie_Pfx_Num(data);
	one_cnt = ((num/4 > 0) ? Count_One(bitmap, 1, num/4) : 0);
	PcTrie_Pfx_Deliver(num, id, length, one_cnt, bitmap, slot_head, node_head);
	// remain to consider if the inherit node should be return back to its original node
	if (num/4 == 0 || bitmap[num/4] == 1)
	{
		PcTrie_Pfx_Add(num, id, length, bitmap, slot_head, node_head);
	}
	else
	{
		if (bitmap[num/4*2] == 1 && bitmap[num/4*2+1] == 1)
			return;
		else
		{	
			PcTrie_Pfx_Add(num, id, length, bitmap, slot_head, node_head);
			for (i = 0; i < 4; i++)
			{
				if (num%4 != i)
					PcTrie_Pfx_Inherit((num/4)*4+i, bitmap, slot_head, node_head);
			}
			if (num/4 != 1 && bitmap[((num/4)%2 > 0 ? num/4-1 : num/4+1)] == 1) 
			{
				if (bitmap[(num/4)/2] == 0)
					return;
				else
				{
					one_cnt = Count_One(bitmap, 1, num/2/4);
					Slot_Delete(slot_head, one_cnt);
					bitmap[num/2/4] = 0;
				}
			}
		}
	}
}

void PcTrie_Pfx_Delete(int num, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head)
{
	t_PcNode *node;
	t_PcSlot *slot;
	int one_cnt;
	if (num > 3 && bitmap[num/4] == 0)
	{
		printf("Error, bitmap bit is 0 and no prefix can be deleted");
		exit(1);
	}
	node = PcNode_Locate(bitmap, slot_head, node_head, num);
	node->id = -1;
	node->len = -1;
//	slot = container_of(node, t_PcNode, pc_node[num%4]);
}

int PcTrie_Pfx_Search(char *data, char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head)
{
	int i = 0;
	int num = 1; 
	int id = -1;
	int one_cnt = 0;
	int bitmap_num, slot_node_order;
	t_PcSlot *slot;
	t_PcNode *node;

	id = node_head[0].id;
	if (strlen(data) != 9)
	{
		printf("Error, The search data length in pctrie isn't 9!\n");
		exit(0);
	}
	while (data[i] != '\0')
	{
		if (data[i] == '0') num = num*2;
		else if (data[i] == '1') num = num*2 + 1;

		if (num <= 3)
		{
			node = PcNode_Locate(bitmap, slot_head, node_head, num);
			if (node->id != -1)
				id = node->id;
		}
		else if (num > 3 && bitmap[num/4] == 1)
		{
			bitmap_num = num/4;
			slot_node_order = num%4;
			one_cnt += Count_One(bitmap, bitmap_num/2 + 1, bitmap_num);
			slot = Slot_Locate(slot_head, one_cnt);
			if (slot->pc_node[slot_node_order].id == -1)/*should use mutex when add a slot 
			and after filled it with id*/
				printf("Error, the id in exist slot shouldn't be -1\n");
			else id = slot->pc_node[slot_node_order].id;
		}
		i++;
	}

	return id;
}


//int main(void)
//{
//	t_PcSlot **slot_head;
//	t_PcNode *node_head, *node;
//	char *bitmap;
//	int id;
//	int num;
//
//	bitmap = Bitmap_Create(9);
//	node_head = PcNode_Create(3);
//	slot_head = Slot_Create();
//
//	PcTrie_Pfx_Add("1", 3, bitmap, slot_head, node_head);
//	PcTrie_Pfx_Add("11", 7, bitmap, slot_head, node_head);
//	PcTrie_Pfx_Add("101", 13, bitmap, slot_head, node_head);
//	PcTrie_Pfx_Add("111", 15, bitmap, slot_head, node_head);
//	PcTrie_Pfx_Add("1010", 26, bitmap, slot_head, node_head);
//	PcTrie_Pfx_Add("1111", 31, bitmap, slot_head, node_head);
//	
//	node = PcNode_Locate(bitmap, slot_head, node_head, 3);
//	id = node->id;
//	printf("the location 3 id is %d\n", id);
//	
//	node = PcNode_Locate(bitmap, slot_head, node_head, 7);
//	id = node->id;
//	printf("the location 7 id is %d\n", id);
//
//	node = PcNode_Locate(bitmap, slot_head, node_head, 13);
//	id = node->id;
//	printf("the location 13 id is %d\n", id);
//	
//	node = PcNode_Locate(bitmap, slot_head, node_head, 15);
//	id = node->id;
//	printf("the location 15 id is %d\n", id);
//
//	node = PcNode_Locate(bitmap, slot_head, node_head, 26);
//	id = node->id;
//	printf("the location 26 id is %d\n", id);
//	
//	node = PcNode_Locate(bitmap, slot_head, node_head, 31);
//	id = node->id;
//	printf("the location 31 id is %d\n", id);
//
//	PcTrie_Pfx_Delete("1010", bitmap, slot_head, node_head);
//	node = PcNode_Locate(bitmap, slot_head, node_head, 26);
//	id = node->id;
//	printf("after delete, the location 26 id is %d\n", id);
//
//}

//int main(void)
//{
//	t_PcSlot **slot_head, *addr1, *addr2, *addr3, *pddr2, *pddr3;
//	t_PcNode *node, *node1, *node2;
//	char *bitmap; 
//	int cnt, num;
//	bitmap = Bitmap_Create(9);
//	bitmap[1] = '1';
//	bitmap[2] = '0';
//	bitmap[3] = '1';
//	bitmap[4] = '1';
//	cnt = Count_One(bitmap, 4);
//	printf("the number of 1 in bitmap is %d\n", cnt);
//
//	num = PcTrie_Pfx_Num("1111");
//	printf("the num of 1111 is %d\n", num);
//
//	node = PcNode_Create(3);
//	slot_head = Slot_Create();
//	printf("slot head is %p\n", slot_head);
//	addr1 = Slot_Add(slot_head, 1);
//	addr2 = Slot_Add(slot_head, 2);
//	addr3 = Slot_Add(slot_head, 3);
//	printf("the addr is %p, %p, %p\n", addr1, addr2, addr3);
//	pddr2 = addr1->next;
//	pddr3 = addr2->next;
//	printf("the addr is %p, %p, %p\n", addr1, pddr2, pddr3);
//
////	Slot_Delete(slot_head, 2);
////	addr1 = *slot_head;
////	addr2 = addr1->next;
////	printf("the addr delete 2 is %p, %p\n", addr1, addr2);
////
////	addr1 = Slot_Locate(slot_head, 1);
////	addr2 = Slot_Locate(slot_head, 2);
////	printf("the locate addr1 and 2 is %p, %p\n", addr1, addr2);
////	addr3 = Slot_Locate(slot_head, 3);
////	printf("the locate addr 3 is %p\n", addr3);
//
//	node1 = PcNode_Locate(bitmap, slot_head, node, 14);
//	node2 = &addr2->pc_node[2];
//	printf("node1 is %p node2 is %p\n", node1, node2);
//
//	PcTrie_Pfx_Add("11111", 23);	

//}
