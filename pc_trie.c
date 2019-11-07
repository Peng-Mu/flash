#include "pc_trie.h"
#include "utils.h"


int IsPairListEmpty(PairList* pairList);
TcError addPairListNode(PairList* pairList, int first, int second);
PairNode* findNodeOfPairList(PairList* pairList, int first);

TcError CreatePcTrie(pc_Trie **pcTrie, NHIModuleSet* lpmModuleSet, int rootId, int rootModuleId, NHITableEntry* rootEntryIndex)
{
	int i,j;
	NHITableEntry *EntryIndex = NULL;
	TcError rc = TcE_OK;
	//if (rootEntryIndex == NULL) return TcE_Failed;
	if(AllocatNHITable(lpmModuleSet, &EntryIndex, &i)!= TcE_OK) 
		return TcE_Failed;
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
		EntryIndex->parentId[i] = -1;
	}
	for (i = 1; i < PC_MOUDLE_SIZE; i++) {
		for (j = 0; j < 1 << COMPRESS_LYAER_NUMBER; j++) {
			(EntryIndex + i)->lpmRecordId[j] = -1;
			(EntryIndex + i)->parentId[j] = -1;
		}
		memset((EntryIndex + i)->individualMark, 0, sizeof((EntryIndex + i)->individualMark));
	}
	ptr->firstNodeLenthList.size = 0;
	ptr->relationList.size = 0;
	ptr->FirstAvailableLocation = 1;
	addPairListNode(&(ptr->firstNodeLenthList), ptr->root.lpmRecordId, 0);
	*pcTrie = ptr;
	return rc;
}
TcError InsertRecord(pc_Trie *pcTrie, pc_TrieNode *pcTrieNode)
{
	int i = 0, j = 0, k = 0, m = 0, item = 0, index = 0, offset = 0, temp = 0, size = 0, temp1 = 0;
	TcError rc = TcE_OK;
	NHITableEntry* currentPtr;
	int *recordId = NULL;
	PairNode *pairNode1 = NULL, *pairNode2 = NULL;
	int layer = (pcTrieNode->lenth - COMPRESS_LYAER_NUMBER > 0) ? pcTrieNode->lenth - COMPRESS_LYAER_NUMBER : 0;
	for (i = 0; i < pcTrieNode->lenth && pcTrieNode->lenth - i > COMPRESS_LYAER_NUMBER; i++) {
		BitBufferGetBit(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), pcTrieNode->lenth - i - 1, &j);
		index = (index << 1) + j;
	}
	j = i;
	i = 0;
	currentPtr = pcTrie->Index[(1 << layer) - 1 + index];
	BitBufferGetBits(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), 0, pcTrieNode->lenth - j, &offset, sizeof(offset));
	if (IsChildFull(pcTrie, (1 << layer) - 1 + index)) {
		RecoveryData(pcTrie, (1 << layer) - 1 + index, &recordId);
		if (!recordId) { rc = TcE_Failed; goto done; }
		if (pcTrieNode->lenth <= COMPRESS_LYAER_NUMBER) {
			for (i = 0; i < 1 << COMPRESS_LYAER_NUMBER; i++) {
				BitBufferGetBits(&i, sizeof(&i), COMPRESS_LYAER_NUMBER - pcTrieNode->lenth, pcTrieNode->lenth, &temp, sizeof(temp));
				if (temp == offset) {
					if (!findNodeOfPairList(&(pcTrie->firstNodeLenthList), pcTrieNode->lpmRecordId)) {
						addPairListNode(&(pcTrie->firstNodeLenthList), pcTrieNode->lpmRecordId, pcTrieNode->lenth);
					}
					pairNode1 = findNodeOfPairList(&(pcTrie->firstNodeLenthList), recordId[i]);
					if (pairNode1->second < pcTrieNode->lenth) {
						if (!findNodeOfPairList(&(pcTrie->relationList), pcTrieNode->lpmRecordId)) {
							addPairListNode(&(pcTrie->relationList), pcTrieNode->lpmRecordId, recordId[i]);
						}
						UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, pcTrieNode->lpmRecordId, recordId[i], i);
						pairNode2 = findNodeOfPairList(&(pcTrie->relationList), recordId[i]);
						if(pairNode2){
							UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, recordId[i], pairNode2->second, i);
						} else {
							UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, recordId[i], -1, i);
						}
						
					} else {
						k = recordId[i];
						m = 0;
						do{
							pairNode2 = findNodeOfPairList(&(pcTrie->relationList), k);
							pairNode1 = findNodeOfPairList(&(pcTrie->firstNodeLenthList), pairNode2->second);
							if (pairNode1->second < pcTrieNode->lenth) {
								if (m == 0) {
									UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, pcTrieNode->lpmRecordId, pairNode1->first, i);
								}
								if (!findNodeOfPairList(&(pcTrie->relationList), pcTrieNode->lpmRecordId)) {
									pairNode2->first = pcTrieNode->lpmRecordId;
									addPairListNode(&(pcTrie->relationList), k, pcTrieNode->lpmRecordId);
								} else {
									pairNode2->second = pcTrieNode->lpmRecordId;
								}
								break;
							}
							k = pairNode2->second;
							m++;
						} while (pairNode1->second > pcTrieNode->lenth);
					}
				}
			}
		} else {
			if (!findNodeOfPairList(&(pcTrie->relationList), pcTrieNode->lpmRecordId)) {
				addPairListNode(&(pcTrie->relationList), pcTrieNode->lpmRecordId, recordId[offset]);
			}
			UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, pcTrieNode->lpmRecordId, recordId[offset], offset);
			pairNode2 = findNodeOfPairList(&(pcTrie->relationList), recordId[offset]);
			if (pairNode2) { UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, recordId[offset], pairNode2->second, offset); 
			} else {
				UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, recordId[offset], -1, offset);
			}
		}
	} else {
		if (currentPtr == NULL) {
			rc = GetNHITableLocation(pcTrie, &(pcTrie->Index[(1 << layer) - 1 + index]));
			currentPtr = pcTrie->Index[(1 << layer) - 1 + index];
			if (rc != TcE_OK) { goto done; }
		}
		BitBufferGetBits(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), 0, pcTrieNode->lenth - j, &offset, sizeof(offset));
		if (pcTrieNode->lenth <= COMPRESS_LYAER_NUMBER) {
			for (i = 0; i < 1 << COMPRESS_LYAER_NUMBER; i++) {
				BitBufferGetBits(&i, sizeof(&i), COMPRESS_LYAER_NUMBER - pcTrieNode->lenth, pcTrieNode->lenth, &temp, sizeof(temp));
				if (temp == offset) {
					if (!findNodeOfPairList(&(pcTrie->firstNodeLenthList), pcTrieNode->lpmRecordId)){
						addPairListNode(&(pcTrie->firstNodeLenthList), pcTrieNode->lpmRecordId, pcTrieNode->lenth);
					}
					pairNode1 = findNodeOfPairList(&(pcTrie->firstNodeLenthList), currentPtr->lpmRecordId[i]);
					if (pairNode1->second < pcTrieNode->lenth){
						if (currentPtr->parentId[i] != -1){
							if (!findNodeOfPairList(&(pcTrie->relationList), currentPtr->lpmRecordId[i])) {
								addPairListNode(&(pcTrie->relationList), currentPtr->lpmRecordId[i], currentPtr->parentId[i]);
							}
						}
						UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, pcTrieNode->lpmRecordId, currentPtr->lpmRecordId[offset], offset);
						UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, currentPtr->lpmRecordId[offset], currentPtr->parentId[offset], offset);
						currentPtr->parentId[i] = currentPtr->lpmRecordId[i];
						currentPtr->lpmRecordId[i] = pcTrieNode->lpmRecordId;
					}
					else {
						pairNode2 = findNodeOfPairList(&(pcTrie->firstNodeLenthList), currentPtr->parentId[i]);
						if (pairNode2->second < pcTrieNode->lenth) {
							if (!findNodeOfPairList(&(pcTrie->relationList), pcTrieNode->lpmRecordId)) {
								addPairListNode(&(pcTrie->relationList), pcTrieNode->lpmRecordId, currentPtr->parentId[i]);
							}
							UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, pcTrieNode->lpmRecordId, currentPtr->parentId[offset], offset);
							currentPtr->parentId[i] = pcTrieNode->lpmRecordId;
						} else {
							k = currentPtr->parentId[i];
							do {
								pairNode2 = findNodeOfPairList(&(pcTrie->relationList), k);
								pairNode1 = findNodeOfPairList(&(pcTrie->firstNodeLenthList), pairNode2->second);
								if (pairNode1->second < pcTrieNode->lenth) {
									if (!findNodeOfPairList(&(pcTrie->relationList), pcTrieNode->lpmRecordId)) {
										pairNode2->first = pcTrieNode->lpmRecordId;
										addPairListNode(&(pcTrie->relationList), k, pcTrieNode->lpmRecordId);
									}
									else {
										pairNode2->second = pcTrieNode->lpmRecordId;
									}
									break;
								}
								k = pairNode2->second;
							} while (pairNode1->second > pcTrieNode->lenth);
						}
					}
				}
			}
			if (pcTrieNode->lenth == COMPRESS_LYAER_NUMBER) {
				BitBufferSetBit(currentPtr->individualMark, sizeof(currentPtr->individualMark), offset);
			}
		} else {
			if (currentPtr->parentId[offset] != -1) {
				if (!findNodeOfPairList(&(pcTrie->relationList), currentPtr->lpmRecordId[offset])) {
					addPairListNode(&(pcTrie->relationList), currentPtr->lpmRecordId[offset], currentPtr->parentId[offset]);
				}
			}
			UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, pcTrieNode->lpmRecordId, currentPtr->lpmRecordId[offset], offset);
			UpdatePcTrieChildNode(pcTrie, (1 << layer) - 1 + index, currentPtr->lpmRecordId[offset], currentPtr->parentId[offset], offset);
			currentPtr->parentId[offset] = currentPtr->lpmRecordId[offset];
			currentPtr->lpmRecordId[offset] = pcTrieNode->lpmRecordId;
			BitBufferSetBit(currentPtr->individualMark, sizeof(currentPtr->individualMark), offset);
		}
	}
	//update parent
	temp1 = (1 << layer) - 1 + index;
	index = 0;
	for (i = 0; i < j; i++) {
		offset = 0;
		BitBufferGetBit(pcTrieNode->recordData, sizeof(pcTrieNode->recordData), pcTrieNode->lenth - i - 1, &k);
		index = index * 2 + 1 + k;
		size = (1 << (COMPRESS_LYAER_NUMBER - 1)) + (k << (COMPRESS_LYAER_NUMBER - 1));
		temp = 0;
		if (!IsChildFull(pcTrie, index)) {
			if (pcTrie->Index[index] == NULL) {
				GetNHITableLocation(pcTrie, &(pcTrie->Index[index]));
				temp = 1;
			}
			if (pcTrie->Index[(index + 1) / 2 - 1] && (temp || temp1== index)) {
				for (m = k << (COMPRESS_LYAER_NUMBER - 1); m < size; m++) {
					BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
					if (item == 0) {
						pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
						pcTrie->Index[index]->parentId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->parentId[m];
					}
					else {
						pcTrie->Index[index]->parentId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
					}
					offset++;
					BitBufferGetBit(pcTrie->Index[index]->individualMark, sizeof(pcTrie->Index[index]->individualMark), offset, &item);
					if (item == 0) {
						pcTrie->Index[index]->lpmRecordId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
						pcTrie->Index[index]->parentId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->parentId[m];
					}
					else {
						pcTrie->Index[index]->parentId[offset] = pcTrie->Index[(index + 1) / 2 - 1]->lpmRecordId[m];
					}
					offset++;
				}
				if (IsChildFull(pcTrie, (index + 1) / 2 - 1)) {
					DeletePcTrieNode(pcTrie, (index + 1) / 2 - 1);
				}
			}
		}
	}
done:
	if (recordId) {
		free(recordId);
	}
	return rc;
}
TcError DeleteRecord(pc_Trie *pcTrie, int lpmId)
{

}
TcError SearchRecord(pc_Trie *pcTrie, int lenth, void *key, int *id) {
	TcError rc = TcE_OK;
	int i = 0, j = 0, k = 0, m = 0, index = 0, location = 0;
	int validLenth = 0;
	int *recordId = NULL;
	PairNode *node = NULL;
	PairNode *node1 = NULL;
	if (lenth > PC_TRIE_BITS) {
		validLenth = PC_TRIE_BITS;
	}
	else { validLenth = lenth; }
	int layer = (validLenth - COMPRESS_LYAER_NUMBER > 0) ? validLenth - COMPRESS_LYAER_NUMBER : 0;
	for (i = 0; i < validLenth && validLenth - i > COMPRESS_LYAER_NUMBER; i++) {
		BitBufferGetBit(key, sizeof(key), validLenth - i - 1, &j);
		index = (index << 1) + j;
	}
	index = (1 << layer) - 1 + index;
	if (validLenth <= COMPRESS_LYAER_NUMBER) {
		if (pcTrie->Index[index]) { 
			recordId = pcTrie->Index[index]->lpmRecordId; 
		} else {
			RecoveryData(pcTrie, 0, &recordId);
			k = 1;
		}
		if (validLenth == COMPRESS_LYAER_NUMBER) {
			BitBufferGetBits(key, sizeof(key), 0, COMPRESS_LYAER_NUMBER, &location, sizeof(location));
			*id = recordId[location];
			goto done;
		}
		i = pcTrie->root.lpmRecordId;
		BitBufferGetBits(key, sizeof(key), 0, validLenth, &location, sizeof(location));
		location = location << (COMPRESS_LYAER_NUMBER - validLenth);
		for (j = location; j < location + 1 << (COMPRESS_LYAER_NUMBER - validLenth); j++) {
			node = findNodeOfPairList(&(pcTrie->firstNodeLenthList), recordId[j]);
			if (!node) { rc = TcE_Failed; goto done; }
			while (node->second > validLenth) {
				node1 = findNodeOfPairList(&(pcTrie->relationList), node->first);
				if (!node1) { rc = TcE_Failed; goto done;}
				node = findNodeOfPairList(&(pcTrie->firstNodeLenthList), node1->second);
				if (!node) { rc = TcE_Failed; goto done; }
			}
			if (node->second > m) {
				m = node->second;
				i = node->first; 
			}
		}
		*id = i;
	} else {
		if (IsChildFull(pcTrie, index)) {
			RecoveryData(pcTrie, 0, &recordId);
			k = 1;
			*id = recordId[location];
			goto done;
		} else {
			for (i = 0; i < validLenth - COMPRESS_LYAER_NUMBER; i++) {
				if (pcTrie->Index[index]) {
					BitBufferGetBits(key, sizeof(key), i, COMPRESS_LYAER_NUMBER, &location, sizeof(location));
					*id = pcTrie->Index[index]->lpmRecordId[location];
					goto done;
				}
				else {
					index = (index + 1) / 2 - 1;
				}
			}
		}
	}
done:
	if (k) { free(recordId); }
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
	int i = 0;
	if (pcTrie->Index[index]->available == 1){
		for (i = 0; i < 1 << COMPRESS_LYAER_NUMBER; i++) {
			if (pcTrie->Index[index]->parentId[i] != -1 && !findNodeOfPairList(&(pcTrie->relationList), pcTrie->Index[index]->lpmRecordId[i])) {
				addPairListNode(&(pcTrie->relationList), pcTrie->Index[index]->lpmRecordId[i], pcTrie->Index[index]->parentId[i]);
			}
		}
		memset(pcTrie->Index[index], 0, sizeof(pcTrie->Index[index]));
	}
	pcTrie->Index[index] = NULL;
	return TcE_OK;
}
int IsChildFull(pc_Trie *pcTrie, int index) {
	if (index * 2 + 2 >= (1 << (PC_TRIE_BITS - COMPRESS_LYAER_NUMBER + 1))) { 
		return 0;
	}
	int i = 1, j = 1;
	if (pcTrie->Index[index * 2 + 1] == NULL) {
		i = IsChildFull(pcTrie, index * 2 + 1);
	}
	if (pcTrie->Index[index * 2 + 2] == NULL) {
		j = IsChildFull(pcTrie, index * 2 + 2);
	}
	return i&j;
}
void RecoveryData(pc_Trie *pcTrie, int index, int **recordId) {
	if (index * 2 + 2 >= (1 << (PC_TRIE_BITS - COMPRESS_LYAER_NUMBER + 1))) {
		if (pcTrie->Index[index]) *recordId = pcTrie->Index[index]->lpmRecordId;
		else *recordId = NULL;
		return;
	}
	int i = 0, j = 0, k = 0, m = 0;
	int *recordId1;
	int *recordId2;
	PairNode *parent1, *parent2;
	
	if (pcTrie->Index[index * 2 + 1] == NULL){
		RecoveryData(pcTrie, index * 2 + 1, &recordId1);
		if (recordId1 == NULL) {
			*recordId = NULL; return;
		}
	} else {
		recordId1 = pcTrie->Index[index * 2 + 1]->lpmRecordId;
		j = 1;
	}
	if (pcTrie->Index[index * 2 + 2] == NULL) {
		RecoveryData(pcTrie, index * 2 + 2, &recordId2);
		if (recordId2 == NULL) {
			*recordId = NULL; return;
		}
	} else {
		recordId2 = pcTrie->Index[index * 2 + 2]->lpmRecordId;
		m = 1;
	}
	int * recordId3 = (int*)malloc((1 << COMPRESS_LYAER_NUMBER)*(sizeof(int)));
	for (k = 0; k < 1 << (COMPRESS_LYAER_NUMBER); k++) { recordId3[k] = -1; }
	if (j) {
		for (k = 0; k < 1 << (COMPRESS_LYAER_NUMBER - 1); k++) {
			if (recordId1[k * 2] == recordId1[k * 2 + 1]) { recordId3[k] = recordId1[k * 2]; }
			else {
				if (pcTrie->Index[index * 2 + 1]->parentId[k * 2] == pcTrie->Index[index * 2 + 1]->parentId[k * 2 + 1]) {
					recordId3[k] = pcTrie->Index[index * 2 + 1]->parentId[k * 2];
				} else if (pcTrie->Index[index * 2 + 1]->parentId[k * 2] == recordId1[k * 2 + 1]) {
					recordId3[k] = recordId1[k * 2 + 1];
				} else {
					recordId3[k] = recordId1[k * 2];
				}
			}
		}
	} else {
		for (k = 0; k < 1 << (COMPRESS_LYAER_NUMBER - 1); k++) {
			if (recordId1[k * 2] == recordId1[k * 2 + 1]) { recordId3[k] = recordId1[k * 2]; }
			else {
				parent1 = findNodeOfPairList(&(pcTrie->relationList), recordId1[k * 2]);
				if (!parent1) { free(recordId3); recordId3 = NULL; goto done; }
				parent2 = findNodeOfPairList(&(pcTrie->relationList), recordId1[k * 2 + 1]);
				if (!parent2) { free(recordId3); recordId3 = NULL; goto done; }
				if (parent1->second == parent2->second) {
					recordId3[k] = parent1->second;
				} else if (parent1->second == recordId1[k * 2 + 1]) {
					recordId3[k] = recordId1[k * 2 + 1];
				} else {
					recordId3[k] = recordId1[k * 2];
				}
			}
		}
	}
	if (m) {
		for (k = 0; k < 1 << (COMPRESS_LYAER_NUMBER - 1); k++) {
			if (recordId2[k * 2] == recordId2[k * 2 + 1]) { recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1))+k] = recordId2[k * 2]; }
			else {
				if (pcTrie->Index[index * 2 + 2]->parentId[k * 2] == pcTrie->Index[index * 2 + 2]->parentId[k * 2 + 1]) {
					recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1)) + k] = pcTrie->Index[index * 2 + 2]->parentId[k * 2];
				}
				else if (pcTrie->Index[index * 2 + 2]->parentId[k * 2] == recordId2[k * 2 + 1]) {
					recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1)) + k] = recordId2[k * 2 + 1];
				}
				else {
					recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1)) + k] =  recordId2[k * 2];
				}
			}
		}
	}
	else {
		for (k = 0; k < 1 << (COMPRESS_LYAER_NUMBER - 1); k++) {
			if (recordId2[k * 2] == recordId2[k * 2 + 1]) { recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1)) + k] = recordId2[k * 2]; }
			else {
				parent1 = findNodeOfPairList(&(pcTrie->relationList), recordId2[k * 2]);
				if (!parent1) { free(recordId3); recordId3 = NULL; goto done; }
				parent2 = findNodeOfPairList(&(pcTrie->relationList), recordId2[k * 2 + 1]);
				if (!parent2) { free(recordId3); recordId3 = NULL; goto done; }
				if (parent1->second == parent2->second) {
					recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1)) + k] = parent1->second;
				} else if (parent1->second == recordId2[k * 2 + 1]) {
					recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1)) + k] = recordId2[k * 2 + 1];
				} else {
					recordId3[(1 << (COMPRESS_LYAER_NUMBER - 1)) + k] = recordId2[k * 2];
				}
			}
		}
	}
done:
	*recordId = recordId3;
	if (j == 0) { free(recordId1); }
	if (m == 0) { free(recordId2); }
	return;
}
TcError UpdatePcTrieChildNode(pc_Trie* pcTrie, int index, int now, int src, int location)
{
	int size, i, j, n1, n2, p1, p2, k, offset, temp;
	char flag;
	TcError rc = 0;
	NHITableEntry* nhi;
	PairNode* pN;
	PairNode *node;
	size = 1 << (PC_TRIE_BITS - COMPRESS_LYAER_NUMBER + 1);
	temp = index;

	i = 2 * temp+1;
	if (i+1 < size)
	{
		if (2 * location < 1 << COMPRESS_LYAER_NUMBER)
		{
			nhi = pcTrie->Index[i];
			offset = 2 * location;
		}
		else if (2 * location >= 1 << COMPRESS_LYAER_NUMBER && 2 * location < 1 << (COMPRESS_LYAER_NUMBER + 1))
		{
			if (i + 1 < size)
			{
				nhi = pcTrie->Index[i + 1];
				offset = 2 * location - (1 << COMPRESS_LYAER_NUMBER);
				i++;
			}
		}
		if (nhi == NULL)
		{
			for (j = 0; j < 2; j++)
			{
				offset = offset + j;
				UpdatePcTrieChildNode(pcTrie, i, now, src, offset);
			}
		}
		else
		{
			for (k = 0; k < 2; k++)
			{
				offset = offset + k;
				n1 = nhi->lpmRecordId[offset];
				p1 = nhi->parentId[offset];
				BitBufferGetBit(nhi->individualMark, sizeof(nhi->individualMark), offset, &flag);
				if (flag == 0)
				{
					if (n1 == src)
					{
						nhi->lpmRecordId[offset] = now;
						UpdatePcTrieChildNode(pcTrie, i, now, src, offset);
					}
					if (p1 == src) {
						nhi->parentId[offset] = now;
						node = findNodeOfPairList(&(pcTrie->relationList), n1);
						if (node&&node->second== src) {
							node->second = now;
						}
						UpdatePcTrieChildNode(pcTrie, i, now, src, offset);
					}
				}
				else
				{
					if (p1 == src)
					{
						nhi->parentId[offset] = now;
						node = findNodeOfPairList(&(pcTrie->relationList), n1);
						if (node&&node->second == src) {
							node->second = now;
						}
						UpdatePcTrieChildNode(pcTrie, i, now, src, offset);
					}
				}
			}

		}

	}



done:
	return rc;
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
int IsPairListEmpty(PairList* pairList) {
	if (pairList->size) { return 0; }
	else { return 1; }
}
TcError addPairListNode(PairList* pairList, int first, int second) {
	PairNode *newNode = NULL;
	newNode = (PairNode*)malloc(sizeof(PairNode));
	newNode->next = NULL;
	newNode->first = first;
	newNode->second = second;
	if (IsPairListEmpty(pairList)) {
		pairList->startNode = newNode;
		pairList->endNode = newNode;
		pairList->size++;
		return TcE_OK;
	}
	pairList->endNode->next = newNode;
	pairList->endNode = newNode;
	pairList->size++;
	return TcE_OK;
}
PairNode* findNodeOfPairList(PairList* pairList, int first) {
	if (IsPairListEmpty(pairList)) { return NULL; }
	PairNode* pairListNode = pairList->startNode;
	while (pairListNode) {
		if (pairListNode->first == first) {
			return pairListNode;
		}
		pairListNode = pairListNode->next;
	}
	return NULL;
}