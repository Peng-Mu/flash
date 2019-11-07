

#include "pc_trie.h"

void hash_destroy();
int hash_init();

struct pc_Trie *hash_query(int level, char *hashkey);

//make a connection to prefix and hashElement
int hash_add(int level, char *hashkey,  pc_Trie *pctrie);

void hash_status();

void str2binary(char* str, int width);
void route_str_to_binary(char *route, char *arr);


