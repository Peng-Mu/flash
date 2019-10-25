#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef int  FlashError;

typedef struct s_PrefixInfo {
	char *data;
	int id;
} t_PrefixInfo;

typedef struct s_TrieNode {
	int is_prefix;
//	char bit_cnt;
	int id;
} t_TrieNode;

	
FlashError Trie_Create(t_TrieNode** trienode, int prefix_len);
void Trie_Destroy(t_TrieNode *pNode);
FlashError Trie_Prefix_Locate(t_TrieNode *node, char *data_13, int *last_num,int* locate);
FlashError Trie_Prefix_Add(t_TrieNode *node, char *data_13, int id);
FlashError Trie_Prefix_Delete(t_TrieNode *node, char *data_13);
FlashError  Trie_Prefix_Search(t_TrieNode *node, const char *prefix_data, int* locate);

