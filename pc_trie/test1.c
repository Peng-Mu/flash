#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "pc_trie.h"

void Display_All(char *bitmap, t_PcSlot **slot_head, t_PcNode *node_head)
{
	int i, j;
	t_PcNode *node;
	printf("%d(%d)\n", node_head[0].id, node_head[0].len);
	printf("%d(%d) %d(%d)\n", node_head[1].id, node_head[1].len, node_head[2].id, node_head[2].len);
	for (i = 2; i <= 9; i++)
	{
		for (j = 1<<i; j < 1<<(i+1); j++)
		{
			node = PcNode_Locate(bitmap, slot_head, node_head, j);
			if(node == NULL)
				printf("# ");
			else
				printf("%d(%d) ", node->id, node->len);
		}
		printf(" the %dth level\n", i);
	}
}

//int main(void)
//{
//	t_PcSlot **slot_head;
//	t_PcNode *node_head, *node;
//	char *bitmap;
//	int id;
//	int num;
//	int curr_one_cnt;
//	
//	bitmap = Bitmap_Create(9);
//	node_head = PcNode_Create(3);
//	slot_head = Slot_Create();
//
//	node_head[0].id = 999;
//	node_head[0].len = 0;
//
//	PcTrie_Add_Update("1111", 31, 4, bitmap, slot_head, node_head);
//	PcTrie_Add_Update("1", 3, 1, bitmap, slot_head, node_head);
//	PcTrie_Add_Update("11", 7, 2, bitmap, slot_head, node_head);
//	PcTrie_Add_Update("000", 8, 3, bitmap, slot_head, node_head);
//	PcTrie_Add_Update("111", 15, 3, bitmap, slot_head, node_head);
//	PcTrie_Add_Update("0", 2, 1, bitmap, slot_head, node_head);
//	PcTrie_Add_Update("00", 4, 2, bitmap, slot_head, node_head);
////	PcTrie_Add_Update("01", 5, 2, bitmap, slot_head, node_head);
//	
//	Display_All(bitmap, slot_head, node_head);
//
//	id = PcTrie_Pfx_Search("11100000", bitmap, slot_head, node_head);
//	printf("the match id is %d\n", id);
////	node = PcNode_Locate(bitmap, slot_head, node_head, 3);
////	id = node->id;
////	printf("the location 3 id is %d\n", id);
////	
////	node = PcNode_Locate(bitmap, slot_head, node_head, 13);
////	id = node->id;
////	printf("the location 13 id is %d\n", id);
////	
////	node = PcNode_Locate(bitmap, slot_head, node_head, 15);
////	id = node->id;
////	printf("the location 15 id is %d\n", id);
////
////	node = PcNode_Locate(bitmap, slot_head, node_head, 26);
////	id = node->id;
////	printf("the location 26 id is %d\n", id);
////	
////	node = PcNode_Locate(bitmap, slot_head, node_head, 31);
////	id = node->id;
////	printf("the location 31 id is %d\n", id);
////
////	node = Left_Child(bitmap, slot_head, node_head, 13, 1, &curr_one_cnt);
////	id = node->id;
////	printf("the leftchild of 13 id is %d\n", id);
////
////	node = Right_Child(bitmap, slot_head, node_head, 7, 0, &curr_one_cnt);
////	id = node->id;
////	printf("the rightchild of 7 id is %d\n", id);
////
//////	PcTrie_Pfx_Inherit(8, bitmap, slot_head, node_head);
////
////
////	PcTrie_Add_Update("010", 10, 3, bitmap, slot_head, node_head);
////
////	node = PcNode_Locate(bitmap, slot_head, node_head, 8);
////	id = node->id;
////	printf("After inherit, the location of 8 id is %d\n", id);
////
////
////	PcTrie_Pfx_Delete(26, bitmap, slot_head, node_head);
////	node = PcNode_Locate(bitmap, slot_head, node_head, 26);
////	id = node->id;
////	printf("after delete, the location 26 id is %d\n", id);
////
////
////
//}
//
//
