#ifndef __wholetrie_h__
#define __wholetrie_h__
#include "pc_trie.h"
#include "ftrie.h"

typedef struct s_PcTrie
{
	char *bitmap;
	t_PcSlot **slot_head;
	t_PcNode *pcnode_head;
} t_PcTrie;

void WholeTrie_Create(t_TrieNode **node, t_PcTrie **trie_ptr);
void WholeTrie_Destroy(t_TrieNode *node, t_PcTrie *trie_ptr);
void WholeTrie_Pfx_Add(char *data, int id, int length, t_TrieNode *node, t_PcTrie *trie_ptr);
int WholeTrie_Pfx_Search(char *data, t_TrieNode *node, t_PcTrie *trie_ptr);

#endif
