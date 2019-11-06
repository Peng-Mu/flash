#include <stdio.h>
#include "wholetrie.h"
#include "pc_trie.h"
#include "ftrie.h"

int main(void)
{
	int num, id;
	t_TrieNode *trie_array_head;
	t_PcTrie *pctrie_hash_head;
	WholeTrie_Create(&trie_array_head, &pctrie_hash_head);
//	num = str2num("1111111111111");
//	printf("the num is %d\n", num);

	WholeTrie_Pfx_Add("11111", 2, 5, trie_array_head, pctrie_hash_head);
	WholeTrie_Pfx_Add("1111111111111", 3, 13, trie_array_head, pctrie_hash_head);
	WholeTrie_Pfx_Add("111111111", 4, 9, trie_array_head, pctrie_hash_head);
	WholeTrie_Pfx_Add("111111111111111", 88, 15, trie_array_head, pctrie_hash_head);
	
	id = WholeTrie_Pfx_Search("1111111110111111111111", trie_array_head, pctrie_hash_head);
	printf("the prefix id is %d\n", id);
	WholeTrie_Destroy(trie_array_head, pctrie_hash_head);
}
