#include "pc_trie.h"
#include "utils.h"
#include "LList.h"

TcError CreatePcTrie(pc_Trie **pcTrie, NHIModuleSet* lpmModuleSet, int rootId, int rootModuleId, NHITableEntry* rootEntryIndex)
{
	int i,j;
	NHITableEntry *EntryIndex = NULL;
	TcError rc = TcE_OK;
	//if (rootEntryIndex == NULL) return TcE_Failed;
	if(AllocatNHITable(lpmModuleSet, &EntryIndex, &i)!= TcE_OK) return NULL;
	pc_Trie *ptr = malloc(sizeof(pc_Trie));
	ptr->root.lpmRecordId = rootId;
	ptr->root.moduleId = rootModuleId;
	ptr->root.rootEntryIndex = rootEntryIndex;
	ptr->moduleId = i;
	ptr->NHITable = EntryIndex;
	memset(ptr->Index, 0, sizeof(ptr->Index));
	ptr->Index[0] = EntryIndex;
	EntryIndex->available = 1;
	for (i = 0; i < 1 << COMPRESS_LYAER_NUMBER; i++){
		EntryIndex->lpmRecordId[i] = ptr->root.lpmRecordId;
	}
	for (i = 1; i < PC_MOUDLE_SIZE; i++) {
		for (j = 0; j < 1 << COMPRESS_LYAER_NUMBER; j++) {
			(EntryIndex + i)->lpmRecordId[j] = -1;
		}
		memset((EntryIndex + i)->individualMark, 0, sizeof((EntryIndex + i)->individualMark));
	}
	ptr->FirstAvailableLocation = 1;
	*pcTrie = ptr;
	return rc;
}
TcError InsertPcTrieNode(pc_Trie *pcTrie, pc_TrieNode *pcTrieNode)
{
	int i = 0, j = 0, k = 0, m = 0, item = 0, index = 0, offset = 0, temp = 0, size = 0;
	TcError rc = TcE_OK;
	NHITableEntry* currentPtr;
	int layer = (pcTrieNode->lenth - COMPRESS_LYAER_NUMBER > 0) ? pcTrieNode->lenth - COMPRESS_LYAER_NUMBER : 0;
	for (i = 0; i < pcTrieNode->lenth && pcTrieNode->lenth - i > COMPRESS_LYAER_NUMBER; i++) {
		BitBufferGetBit(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), pcTrieNode->lenth - i - 1, &j);
		index = (index << 1) + j;
	}
	j = i;
	currentPtr = pcTrie->Index[(1 << layer) - 1 + index];
	BitBufferGetBits(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), 0, pcTrieNode->lenth - j, &offset, sizeof(offset));
	if (IsChildFull(pcTrie, (1 << layer) - 1 + index)) {	
		if (pcTrieNode->lenth < COMPRESS_LYAER_NUMBER) {
			for (i = 0; i < 1 << COMPRESS_LYAER_NUMBER; i++) {
				BitBufferGetBits(&i, sizeof(&i), COMPRESS_LYAER_NUMBER - pcTrieNode->lenth, pcTrieNode->lenth, &temp, sizeof(temp));
				if (temp == offset) {
					BitBufferGetBit(currentPtr->individualMark, sizeof(currentPtr->individualMark), i, &item);
					if (item == 1) { continue; }
					currentPtr->lpmRecordId[i] = pcTrieNode->lpmRecordId;
				}
			}
		}
		else {
			k = index * 2 + offset / (1<<(COMPRESS_LYAER_NUMBER-1));
			
			currentPtr->lpmRecordId[offset] = pcTrieNode->lpmRecordId;
			BitBufferSetBit(currentPtr->individualMark, sizeof(currentPtr->individualMark), offset);
		}
	} else {
		if (currentPtr == NULL) {
			rc = GetNHITableLocation(pcTrie, &(pcTrie->Index[(1 << layer) - 1 + index]));
			currentPtr = pcTrie->Index[(1 << layer) - 1 + index];
			if (rc != TcE_OK) { goto done; }
		}
		BitBufferGetBits(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), 0, pcTrieNode->lenth - j, &offset, sizeof(offset));
		if (pcTrieNode->lenth < COMPRESS_LYAER_NUMBER) {
			for (i = 0; i < 1 << COMPRESS_LYAER_NUMBER; i++) {
				BitBufferGetBits(&i, sizeof(&i), COMPRESS_LYAER_NUMBER - pcTrieNode->lenth, pcTrieNode->lenth, &temp, sizeof(temp));
				if (temp == offset) {
					BitBufferGetBit(currentPtr->individualMark, sizeof(currentPtr->individualMark), i, &item);
					if (item == 1) { continue; }
					currentPtr->lpmRecordId[i] = pcTrieNode->lpmRecordId;
				}
			}
		} else {
			currentPtr->lpmRecordId[offset] = pcTrieNode->lpmRecordId;
			BitBufferSetBit(currentPtr->individualMark, sizeof(currentPtr->individualMark), offset);
		}
	}
	//update parent
	index = 0;
	for (i = 0; i < j; i++) {
		offset = 0;
		BitBufferGetBit(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), pcTrieNode->lenth - i - 1, &k);
		index += (1 << (i + 1)) - 1 + k;
		if (pcTrie->Index[index] == NULL&&(!IsChildFull(pcTrie, index))) {
			GetNHITableLocation(pcTrie, &(pcTrie->Index[index]));
			size = (1 << (COMPRESS_LYAER_NUMBER - 1)) + (k << (COMPRESS_LYAER_NUMBER - 1));
			for (m = k << (COMPRESS_LYAER_NUMBER - 1); m < size; m++) {
				BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
				if (item == 0) {
					pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
				}
				offset++;
				BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
				if (item == 0) {
					pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
				}
				offset++;
			}
		} else if (pcTrie->Index[index]&&IsChildFull(pcTrie, index)){
			DeletePcTrieNode(pcTrie, index);
		}
	}
	//update children
	UpdatePcTrieChildNode(pcTrie, index);
done:
	return rc;
}
TcError GetNHITableLocation(pc_Trie *pcTrie, NHITableEntry **NHITableIndex)
{
	if (pcTrie->FirstAvailableLocation == -1) {
		return TcE_Alloc_Fail_Space;
	}
	NHITableEntry *currentPtr = pcTrie->NHITable + pcTrie->FirstAvailableLocation;
	int i = 0, j = 0;
	j = pcTrie->FirstAvailableLocation;
	currentPtr->available = 1;
	*NHITableIndex = currentPtr;
	for (i = pcTrie->FirstAvailableLocation + 1; i % PC_MOUDLE_SIZE != pcTrie->FirstAvailableLocation; i++) {
		if ((pcTrie->NHITable + (i % PC_MOUDLE_SIZE))->available == 0) {
			pcTrie->FirstAvailableLocation = i % PC_MOUDLE_SIZE;
			break;
		}
	}
	if (i % PC_MOUDLE_SIZE == j) { pcTrie->FirstAvailableLocation = -1; }
	
	return TcE_OK;
}
TcError AllocatNHITable(NHIModuleSet* lpmModuleSet, NHITableEntry **NHITableIndex, int *moduleId)
{
	int i, j;
	if (lpmModuleSet == NULL) return TcE_Failed;
	for (i = 0; i < PC_MOUDLE_COUNT; i++) {
		if (lpmModuleSet->markOfModuleAvailable[i] == 0) {
			lpmModuleSet->markOfModuleAvailable[i] = 1;
			*NHITableIndex = lpmModuleSet->module[i].entry;
			*moduleId = i;
			for (j = 0; j < PC_MOUDLE_SIZE; j++) {
				(lpmModuleSet->module[i].entry[j]).available = 0;
			}
			return TcE_OK;
		}
	}
	printf("there is no space!\n");
	return TcE_Alloc_Fail_Space;
}
TcError DeletePcTrieNode(pc_Trie *pcTrie, int index) {
	if (pcTrie == NULL) return TcE_Failed;
	if (pcTrie->Index[index]->available == 1){
		memset(pcTrie->Index[index], 0, sizeof(pcTrie->Index[index]));
	}
	pcTrie->Index[index] = NULL;
	return TcE_OK;
}
int IsChildFull(pc_Trie *pcTrie, int index) {

	if (index * 2 + 2 >= (1 << (PC_TRIE_BITS - COMPRESS_LYAER_NUMBER + 1))) { return 0; }
	if (pcTrie->Index[index * 2 + 1] == NULL) {
		return IsChildFull(pcTrie, index * 2 + 1);
	}
	if (pcTrie->Index[index * 2 + 2] == NULL) {
		return IsChildFull(pcTrie, index * 2 + 2);
	}
	return 1;
}
TcError UpdatePcTrieChildNode(pc_Trie *pcTrie, int index)
{
	int size, i, j, k, m, offset, item;
	size = 1 << (PC_TRIE_BITS - COMPRESS_LYAER_NUMBER + 1);
	int buffer[1 << (PC_TRIE_BITS - COMPRESS_LYAER_NUMBER + 1)];
	for (i = 0; i < size; i++) { buffer[i] = -1; }
	i = 0; k = 0;
	buffer[i] = index;
	while (buffer[i%size] != -1) {
		index = buffer[i % size] * 2 + 1;
		if (index + 1 >= size) { buffer[i++ % size] = -1; continue; }
		offset = 0;
		buffer[(++k) % size] = index;
		if (pcTrie->Index[index]) {
			for (m = 0; m < 1 << (COMPRESS_LYAER_NUMBER - 1); m++) {
				BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
				if (item == 0) {
					pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
				}
				offset++;
				BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
				if (item == 0) {
					pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
				}
				offset++;
			}
		}
		buffer[(++k) % size] = index;
		index = buffer[i % size] * 2 + 2;
		if (pcTrie->Index[index]) {
			for (m = 1 << (COMPRESS_LYAER_NUMBER - 1); m < 1 << COMPRESS_LYAER_NUMBER; m++) {
				BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
				if (item == 0) {
					pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
				}
				offset++;
				BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
				if (item == 0) {
					pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
				}
				offset++;
			}
		}
		buffer[i] = -1;
		i++;
	}
}
TcError InitLpmModuleSet(NHIModuleSet** LpmModuleSet) {
	NHIModuleSet *ptr = malloc(sizeof(NHIModuleSet));
	if (ptr == NULL)  return TcE_Failed;
	int i = 0, j = 0;
	for (i = 0; i < PC_MOUDLE_COUNT; i++) {
		ptr->entryCount[i] = 0;
		ptr->markOfModuleAvailable[i] = 0;
	}
	*LpmModuleSet = ptr;
	return TcE_OK;
}
