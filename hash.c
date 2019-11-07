#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "utils.h"


#define trie_number    8
#define LEVEL_0_LENGTH 13
#define PCTRIE_LENGTH  9


typedef struct s_hashElement
{
	struct hashElement* next;
	char* hashkey;
	pc_Trie* pctrie;
	
}hashNode;

hashNode * hash_create( char* hashkey, pc_Trie* pcTrie)
{
	hashNode * hashelement = (hashNode *) malloc(sizeof(hashNode));
	hashelement->next = NULL;
	hashelement->pctrie = pcTrie;
	hashelement->hashkey = malloc(strlen(hashkey)+1);
	memcpy(hashelement->hashkey, hashkey, strlen(hashkey));
	hashkey[strlen(hashkey)]='\0';
	return hashelement;
}

struct hashElement** hashtable[16];

int hash_init()
{
	memset(hashtable, 0, sizeof(hashtable));
}

int hash_list_add(hashNode** st, hashNode* node)
{
	hashNode * p = *st;
	hashNode* prev = p;
	if(p == NULL)
	{
		p = node;
		return 0;
	}
	while(p)
	{
		prev = p;
		p = p->next ;
	}
	prev->next = node;
	return 0;
}

#define hash_table_size 16*1024
int hash1(char*hashkey, int length)
{
	int j = 0 ;
	int hashval = 0 ;
	int bitval = 0 ;
	for(int i = 0 ;  i < length ; i++)
	{
		BEBitBufferGetBit(hashkey,length,i,&bitval);
		if( j == 14)
			j = 0;
		hashval += bitval << j ;
		if( hashval >= 0x3fff)
			hashval = hashval - 0x3fff;
		j++;
	}

	return hashval;
}


struct pc_Trie *hash_query(int level, char *hashkey)
{
	int hashval = 0 ;
	
	if(hashtable[level] == NULL)
	{
		return NULL;
	}
	str2binary(hashkey, strlen(hashkey));
	hashval = hash1(hashkey,strlen(hashkey));
	hashNode* node =  hashtable[level][hashval];
	while(node != NULL)
	{
			if (strcmp(node->hashkey, hashkey) == 0)
				return node->pctrie;
			node= node->next;					
	}

	return NULL;
	


}

void str2binary(char* str, int width)
{
  int i,n;
  char* temp = malloc(BYTE_COUNT(width));
  memset(temp, 0, BYTE_COUNT(width));
  for(i = 0; i < width; i++) {
  		n = i/8;
        if(str[i] == '1') {
            temp[n] |= (1 << (i%8));
        }
    }
  temp[n+1]='\0';
  memset(str, 0, width);
  memcpy(str,temp,strlen(temp));
}
void route_str_to_binary(char *route, char *arr)
{
    int i;
    memset(arr, 0, (strlen(route)+7)/8);
    for(i = 0; i < strlen(route); i++) {
        if(route[i] == '1') {
            arr[i/8] |= (1 << (i%8));
        }
    }
    
}


int hash_add(int level, char *hashkey,  pc_Trie *pctrie)
{
	int hashval = 0 ;
	int rc = 0;
	int length = strlen(hashkey);
	char* hashkey1 = malloc(strlen(hashkey));
	if(hashtable[level] == NULL)
	{
		hashtable[level] = (hashNode**) malloc(hash_table_size*sizeof(void*));
		if(hashtable[level] == NULL) {
			printf("error hashtable, no more space!\n");
			rc= 1;
			goto done;
		}
		memset(hashtable[level],0,sizeof(hashtable[level]));	
	}
	route_str_to_binary(hashkey, hashkey1);
	hashval=hash1(hashkey1,length);
	rc = hash_list_add(&hashtable[level][hashval],hash_create(hashkey,pctrie));

done:
	return rc;
}


void hash_destroy()
{

	hashNode* prev = NULL, *p= NULL;
	for(int i = 0 ; i < 16 ; i++)
	{
			if(hashtable[i] != NULL)
			{
				for(int j = 0 ; j < hash_table_size; j++)
				{
					if(hashtable[i][j] != NULL)
					{
						prev = hashtable[i][j];
						while(prev)
					  {
						p = prev->next;
						free(prev);
						prev = p;
					  }
					}
				}
				free(hashtable[i]);
				
			}
	}
}

void hash_status()
{
	
		int i,j, k, h ,count = 0, hashval_cnt = 0;
		int nhi_cnt = 0, nhi_table_size = 0;
		int record_cnt;
		int pctrie_num = 0, nhi_num = 0, nhi_table_size_total = 0;
		int BITMAP_SIZE = 0;
		k =  trie_number ;
		h = 0 ;
		while( k/2 != 1 && k%2 == 0)
		{
			k = k/2;
			h += 1;
		}
		BITMAP_SIZE = 1 << (PCTRIE_LENGTH - h +1);
		hashNode *he;
		printf("-----------------------------------------------------------------\n");
		printf("stride=%d comp_size=%d\n", PCTRIE_LENGTH ,trie_number);
		printf("level pctrie record hashval nhiNum nhiTabSize\n");
		for(i = 0; i < 16; i++) {
			count = 0;
			hashval_cnt = 0;
			nhi_cnt = 0;
			nhi_table_size = 0;
			record_cnt = 0;
			if(hashtable[i] != NULL) {
				for(j = 0; j < 16*1024; j++) {
					if((he = hashtable[i][j]) == NULL) { continue; }
					hashval_cnt++;
					while(he) {
						count++;
						//nhi_cnt += he->pctrie->nhi_cnt;
						//nhi_table_size += he->pctrie->nhi_table_size;
						//record_cnt += he->pctrie->record_cnt;
						he = he->next;
					}
				}
				printf("%5d %6d %6d %7d %6d %10d\n", i, count, record_cnt, 
					hashval_cnt, nhi_cnt, nhi_table_size);
				pctrie_num += count;
				//nhi_num += nhi_cnt;
				//nhi_table_size_total += nhi_table_size;
			}
		}
		printf("# Assuming that nexthop is 32 bits\n");
		printf("Memory_bitmap = pctrie_num(%d) * bitmap_size(%d) ~= %dMb\n", 
			pctrie_num, BITMAP_SIZE*8, (pctrie_num * BITMAP_SIZE * 8)/(1024*1024));
		/*printf("Memory_nhi = nhi_num(%d) * sizeof_nhi(%d) ~= %dMb\n", 
			nhi_num, 32, nhi_num * 32 /(1024*1024));
		printf("Memory_nhi(actual) = nhi_table_size_total(%d) * sizeof_nhi(%d) ~= %dMb\n", 
			nhi_table_size_total, 32, nhi_table_size_total * 32 /(1024*1024));*/
		printf("-----------------------------------------------------------------\n");
	

}

