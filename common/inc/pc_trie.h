#ifndef __pc_trie_h__
#define __pc_trie_h__

#include "TcError.h"

#define PC_TRIE_BITS 9
#define COMPRESS_LYAER_NUMBER 3
#define PC_TOTAL_SIZE 256 * 16 *1024
#define PC_MOUDLE_SIZE 128
#define PC_MOUDLE_COUNT (PC_TOTAL_SIZE/PC_MOUDLE_SIZE)

struct BitMap;
struct NHIModuleSet;
struct NHITableEntry;
struct sPairList;

typedef struct sPairList {
	struct sPairList* next;
	int first;
	int second;
}PairNode;

typedef struct sNHITableEntry {
	unsigned char available;
	int lpmRecordId[1 << COMPRESS_LYAER_NUMBER];
	unsigned char individualMark[((1 << COMPRESS_LYAER_NUMBER) + 7) / 8];
	int parentId[1<<COMPRESS_LYAER_NUMBER];
	void *AD[1 << COMPRESS_LYAER_NUMBER];
}NHITableEntry;

typedef struct sNHIModuleSet {
	int markOfModuleAvailable[PC_MOUDLE_COUNT];
	int entryCount[PC_MOUDLE_COUNT];
	struct pcModule { 
		NHITableEntry entry[PC_MOUDLE_SIZE];
	}module[PC_MOUDLE_COUNT];
}NHIModuleSet;

typedef struct sRootInfo {
	int lpmRecordId;
	int moduleId;
	NHITableEntry* rootEntryIndex;
}rootInfo;

typedef struct BitMap {
	rootInfo root;
	NHITableEntry* NHITable;
	NHITableEntry* Index[1 << (PC_TRIE_BITS - COMPRESS_LYAER_NUMBER+1)];
	int FirstAvailableLocation;
	int moduleId;
	PairNode* relationList;
	PairNode* lastRelationListNode;
	int relationListSize;
	PairNode* firstNodeLenthList;
}pc_Trie;

typedef struct sPc_TrieNode {
	int lpmRecordId;
	int lenth;
	unsigned char recordData[18];
}pc_TrieNode;

TcError CreatePcTrie(pc_Trie **pcTrie, NHIModuleSet* lpmModuleSet, int rootId, int rootModuleId, NHITableEntry* rootEntryIndex);
TcError InsertPcTrieNode(pc_Trie *pcTrie, pc_TrieNode *pcTrieNode);
TcError AllocatNHITable(NHIModuleSet* lpmModuleSet, NHITableEntry **NHITableIndex, int *moduleId);
TcError GetNHITableLocation(pc_Trie *pcTrie, NHITableEntry **NHITableIndex);
TcError DeletePcTrieNode(pc_Trie *pcTrie, int index);
TcError UpdatePcTrieChildNode(pc_Trie *pcTrie, int index);
void RecoveryData(pc_Trie *pcTrie, int index, int **recordId);
int IsChildFull(pc_Trie *pcTrie, int index);
TcError InitLpmModuleSet(NHIModuleSet** lpmModuleSet);
int IsPairListEmpty(PairNode* pairList) {
	if (pairList->next) { return 0; }
	else { return 1; }
}
TcError addPairListNode(PairNode** lastRelationListNode, int first, int second) {
	PairNode* newNode = malloc(sizeof(PairNode));
	newNode->next = NULL;
	newNode->first = first;
	newNode->second = second;
	(*lastRelationListNode)->next = newNode;
	*lastRelationListNode = newNode;
	return TcE_OK;
}
PairNode* findSecondOfPairList(PairNode* pairList, int first) {
	PairNode* pairListNode = pairList->next;
	while (pairListNode) {
		if (pairListNode->first == first) {
			return pairListNode;
		}
		pairListNode = pairListNode->next;
	}
	return NULL;
}
#endif