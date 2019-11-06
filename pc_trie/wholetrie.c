#include <stdlib.h>
#include <stdio.h>
#include "pc_trie.h"
#include "ftrie.h"
#include "wholetrie.h"

//typedef struct s_PcTrie
//{
//	char *bitmap;
//	t_PcSlot **slot_head;
//	t_PcNode *pcnode_head;
//} t_PcTrie;

int  str2num(char *data_13)
{
	int i = 0;
	int num = 0;
	while(data_13[i] != '\0')
	{
		if (data_13[i] == '1')
			num += 1<<(12-i);
		else if (data_13[i] == '0')
			num += 0;
		i++;
	}
	return num;
}

t_PcTrie* Hash(char *data_13, t_PcTrie *trie_ptr)
{
	int num;
	if (strlen(data_13) < 13)
	{
		printf("the input hash key is shorter than 13!\n");
		exit(1);
	}
	num = str2num(data_13);
	return &trie_ptr[num];
}

void WholeTrie_Create(t_TrieNode **node, t_PcTrie **trie_ptr)
{
	int i;
	int total = 1<<13;
	Trie_Create(node, 13);
	*trie_ptr = (t_PcTrie*) malloc(total*sizeof(t_PcTrie));
	memset(*trie_ptr, 0, total*sizeof(t_PcTrie));
	if (*trie_ptr == NULL)
	{
		printf("t_PcTrie malloc failed!\n");
		exit(1);
	}
	for (i = 0; i < (1<<13); i++)
	{
		(*trie_ptr)[i].bitmap = Bitmap_Create(9);
		(*trie_ptr)[i].slot_head = Slot_Create();
		(*trie_ptr)[i].pcnode_head = PcNode_Create(3);
		(*trie_ptr)[i].pcnode_head[0].id = 999;
		(*trie_ptr)[i].pcnode_head[0].len = 0;
	}

}

void WholeTrie_Destroy(t_TrieNode *node, t_PcTrie *trie_ptr)
{
	int i;
	Trie_Destroy(node);
	for (i = 0; i < 1<<13; i++)
	{
		Bitmap_Destroy(trie_ptr[i].bitmap);
		Slot_Destroy(trie_ptr[i].slot_head);
		PcNode_Destroy(trie_ptr[i].pcnode_head);
	}
	free(trie_ptr);
}

void WholeTrie_Pfx_Add(char *data, int id, int length, t_TrieNode *node, t_PcTrie *trie_ptr)
{
	char data_13[14], data_9[10];
	int i;
	t_PcTrie *pctrie;
	strncpy(data_13, data, 13);
	data_13[13] = '\0';
	Trie_Prefix_Add(node, data_13, id);
	if (strlen(data) > 13)
	{
		i = 13;
		while (data[i] != '\0')
		{
			data_9[i-13] = data[i];
			i++;
		}
		data_9[i-13] = '\0';
		pctrie = Hash(data_13, trie_ptr);
		PcTrie_Add_Update(data_9, id, length, pctrie->bitmap, pctrie->slot_head, pctrie->pcnode_head);
	}
}
		
int WholeTrie_Pfx_Search(char *data, t_TrieNode *node, t_PcTrie *trie_ptr)
{
	char data_13[14], data_9[10];
	int i, result_id, pctrie_id;
	t_PcTrie *pctrie;
	strncpy(data_13, data, 13);
	data_13[13] = '\0';
	for (i = 0; i < 10; i++)
		data_9[i] = data[i+13];
	Trie_Prefix_Search(node, data_13, &result_id);
	pctrie = Hash(data_13, trie_ptr);
	pctrie_id = PcTrie_Pfx_Search(data_9, pctrie->bitmap, pctrie->slot_head, pctrie->pcnode_head);
	if (pctrie_id >= 0 && pctrie_id != 999)
		result_id = pctrie_id;
	return result_id;
}


//int main(void)
//{
//	int num, id;
//	t_TrieNode *trie_array_head;
//	t_PcTrie *pctrie_hash_head;
//	WholeTrie_Create(&trie_array_head, &pctrie_hash_head);
////	num = str2num("1111111111111");
////	printf("the num is %d\n", num);
//
//	WholeTrie_Pfx_Add("11111", 2, 5, trie_array_head, pctrie_hash_head);
//	WholeTrie_Pfx_Add("1111111111111", 3, 13, trie_array_head, pctrie_hash_head);
//	WholeTrie_Pfx_Add("111111111", 4, 9, trie_array_head, pctrie_hash_head);
//	WholeTrie_Pfx_Add("111111111111111", 88, 15, trie_array_head, pctrie_hash_head);
//	
//	id = WholeTrie_Pfx_Search("1111111110111111111111", trie_array_head, pctrie_hash_head);
//	printf("the prefix id is %d\n", id);
//	WholeTrie_Destroy(trie_array_head, pctrie_hash_head);
//}
