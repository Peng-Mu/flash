#ifndef __pc_trie_h__
#define __pc_trie_h__

#include "TcError.h"

#define PC_TRIE_BITS 9
#define COMPRESS_LYAER_NUMBER 3
#define PC_TOTAL_SIZE 256 * 16 *1024
#define PC_MOUDLE_SIZE 64+1
#define PC_MOUDLE_COUNT (PC_TOTAL_SIZE/PC_MOUDLE_SIZE)

struct BitMap;
struct NHITableSet;
struct NHITableEntry;
struct PairList;
struct PairNode;

typedef struct sPairNode {
	struct sPairNode *next;
	int first;
	int second;
}PairNode;

typedef struct sPairList {
	PairNode* startNode;
	PairNode* endNode;
	int size;
}PairList;

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
	PairList relationList;
	PairList firstNodeLenthList;
}pc_Trie;

typedef struct sPc_TrieNode {
	int lpmRecordId;
	int lenth;
	unsigned char recordData[18];
}pc_TrieNode;

TcError CreatePcTrie(pc_Trie **pcTrie, NHIModuleSet* lpmModuleSet, int rootId, int rootModuleId, NHITableEntry* rootEntryIndex);
TcError InsertRecord(pc_Trie *pcTrie, pc_TrieNode *pcTrieNode);
TcError DeleteRecord(pc_Trie *pcTrie, int lpmId);
TcError SearchRecord(pc_Trie *pcTrie, int lenth, void *key, int* id);
TcError AllocatNHITable(NHIModuleSet* lpmModuleSet, NHITableEntry **NHITableIndex, int *moduleId);
TcError GetNHITableLocation(pc_Trie *pcTrie, NHITableEntry **NHITableIndex);
TcError DeletePcTrieNode(pc_Trie *pcTrie, int index);
TcError UpdatePcTrieChildNode(pc_Trie* pcTrie, int index, int now, int src, int location);
void RecoveryData(pc_Trie *pcTrie, int index, int **recordId);
int IsChildFull(pc_Trie *pcTrie, int index);
TcError InitLpmModuleSet(NHIModuleSet** lpmModuleSet);

#endif