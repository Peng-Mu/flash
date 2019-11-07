#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ftrie.h"



	
FlashError Trie_Create(t_TrieNode** trienode, int prefix_len)
{
	

	int i;
	FlashError rc = 0;
	int total_num = 2 << prefix_len+1;
	//printf("total num is %d\n", total_num);
	t_TrieNode* node = (t_TrieNode*) malloc(total_num*sizeof(t_TrieNode));
	if (node == NULL) 
	{ 
		printf("malloc failed!\n"); 
		rc = 1; 
		goto done; 
	}
	memset(node, 0, total_num*sizeof(*node));
	
	for (i = 1; i < total_num; i++) {
		node[i].is_prefix = 0;
		node[i].id = -1;
	}
	
	*trienode = node;
 
done:
	return rc;
}

void Trie_Destroy(t_TrieNode *pNode)
{
	free(pNode);
}

FlashError Trie_Prefix_Locate(t_TrieNode *node, char *data_13, int* last_num, int* locate)
{
	int num = 1;
	int i = 0;
	int last = 1;
	FlashError rc = 0;
	if (strlen(data_13) > 13) { 
		printf("Error, the prefix need to locate is over 13bit\n");
		rc = 1 ;
		goto done;
	}
	while (data_13[i] != '\0') {
		if (node[num].is_prefix == 1) { last = num; }
		if (data_13[i] == '0') { num = 2*num; }
		else if (data_13[i] == '1') { num = 2*num + 1; }
		else { 
			rc = 1 ;
			printf("Error, the num is not 0 or 1!\n");
			goto done;
		}
		i++;
	}
	if (last_num != NULL) { *last_num = last; }
	*locate = num;
done:
	return rc;
}

FlashError Trie_Prefix_Add(t_TrieNode *node, char *data_13, int id)
{
	
	FlashError rc = 0;
	int num = 0;
	
	if (strlen(data_13) > 13) { 
		printf("Error, the prefix need to add is over 13bit\n"); 
		rc = 1;
		goto done;
	}
	
	rc = Trie_Prefix_Locate(node, data_13, NULL,&num);
	if(rc)
	{
		printf("Error, adding cant find the node\n"); 
		goto done;
	}
	node[num].is_prefix = 1;
	node[num].id = id;
done:
	return rc;
}

FlashError Trie_Prefix_Delete(t_TrieNode *node, char *data_13)
{
	FlashError rc = 0;
	int num =0;
	
	if (strlen(data_13) > 13) { 
		printf("Error, the prefix need to delete is over 13bit\n"); 
		rc = 1;
		goto done;
	}
	
	rc = Trie_Prefix_Locate(node, data_13, NULL , &num);
	if(rc)
	{
		printf("Error,deleteing cant find the node\n"); 
		goto done;
	}
	node[num].is_prefix = 0;
	node[num].id = -1;
done:
	return rc;
}

FlashError Trie_Prefix_Search(t_TrieNode *node, const char *prefix_data, int * locate)
{
	char *data = prefix_data;
	char data_13[14];
	int num, last_prefix, id;
	FlashError rc =0 ;
	if (strlen(data) <= 13) {
		rc = Trie_Prefix_Locate(node, data, &last_prefix,&num);
		if(rc)
		{
			printf("Error,searching cant find the node\n"); 
			goto done;
		}
		id = node[num].id;
	}
	else {
		strncpy(data_13, data, 13);
		data_13[13] = '\0';
		rc = Trie_Prefix_Locate(node, data_13, &last_prefix, &num);
		if(rc)
		{
			printf("Error, searching cant find the node\n"); 
			goto done;
		}
		if (node[num].id != -1) { id = node[num].id; }
		else id = node[last_prefix].id; 
	}
	*locate= id;
done:
	return rc;
}

