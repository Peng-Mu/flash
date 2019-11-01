#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "List.h"
#include "interface.h"
#include "utils.h"
#include "hash.h"
#define trie_number    8
#define LEVEL_0_LENGTH 13
#define PCTRIE_LENGTH  9


FlashError FlashTable_Create( sFlashtable** fTable) {
	
    FlashError rc = 0;
    sFlashtable* table = (sFlashtable*)malloc(sizeof(*table));
    if (!table) {
        printf("out-of-memory\n"); 
		rc = 1; 
		goto done;
    }
	memset(table, 0, sizeof(*table));
	
	t_TrieNode* node = NULL;
	rc = Trie_Create(&node, LEVEL_0_LENGTH);
	if(rc)
	{
		printf("create the trie_node failure");
		goto done;
	}
  	table->trienode = node; 


	
    *fTable = table;
done:
    return rc;
}

void* FlashTable_Destory(sFlashtable* fTable) {
	if (fTable->flashrecordList) {
        if (List_Element_Count(fTable->flashrecordList))   {
            sFlashrecord** pRecord = NULL;
            sFlashrecord* record = NULL;
            for (pRecord = (sFlashrecord**)List_Element_First(fTable->flashrecordList);
                pRecord;
                pRecord = (sFlashrecord**)List_Element_Next(fTable->flashrecordList, pRecord))
            {
                record = *pRecord;
                FlashRecord_Destory(record);
                *pRecord = NULL;
            }
        }
        List_Destroy(fTable->flashrecordList);
    }
	if (fTable) free(fTable);
}


FlashError FlashRecord_Create(sFlashrecord** fRecord){
    FlashError rc = 0;
	
    sFlashrecord*  record = (sFlashrecord*)malloc(sizeof(*record));
    if (NULL == record)    {
        printf("out-of-memory\n"); 
        rc = 1; 
		goto done;
    }
    memset(record, 0, sizeof(*record));
    *fRecord = record;
    return TcE_OK;
done:
    if (record)  free(record);
    return rc;
}

void* FlashRecord_Destory(sFlashrecord* flashrecord)
{
	if(flashrecord->ftable->recordCount)
	{
		for(int i = 0 ; i < flashrecord->ftable->recordCount ; i++)
		{
			FlashTable_Record_Rm(flashrecord->ftable, i);
		}
	}
	free(flashrecord);
}

FlashError FlashTable_Record_Add(sFlashtable* flashtable, sFlashrecord* flashrecord, int id)
{
	FlashError rc  = 0;
    sFlashtable*  table  = (sFlashtable*) flashtable;
    sFlashrecord *record = NULL, **pListRecord = NULL;
	int nhi_number = 0;
	int nhi_index  = 0;
	char* hash_key = NULL;
	char* nhi_str  = NULL;
	pc_Trie *pct;
	NHIModuleSet* LpmModuleSet ;
	NHITableEntry* rootEntryIndex ;
	int rootId = 999;
	pc_TrieNode *pcTrieNode = malloc(sizeof(pc_TrieNode));
	pcTrieNode->lpmRecordId = flashrecord->id;
	pcTrieNode->lenth = flashrecord->width;
	memset(pcTrieNode->recordData, 0, sizeof(pcTrieNode->recordData));
	strncpy(pcTrieNode->recordData, flashrecord->prefix , flashrecord->width);
	
	 if (table->depth && table->flashrecordList == NULL)    {
        rc = List_Create(table->depth, sizeof(void*), &table->flashrecordList);
		//printf("%d\n",rc);
		if(rc)
		{
			printf("create list error\n");
			goto done;
	 	}
	 }
	rc = FlashRecord_Create(&record);
	if(rc)
	{
		printf("add record, create record error !!");
		goto done;
	}
	record->ftable = table;
	record->id 	   = flashrecord->id;
	record->width  = flashrecord->width;
	record->prefix = (char*)malloc(BYTE_COUNT(record->width));
	if(record->prefix == NULL)
	{
		printf("copy record fail");
	}
	memcpy(record->prefix, flashrecord->prefix, BYTE_COUNT(record->width));
	//printf("rules: %s \n",  record->prefix );
	if (id == -1) {
        rc = List_Element_Add(table->flashrecordList, (void**)&pListRecord);
		if(rc)
		{
			printf("add the record to list failure");
			goto done;
		}
        rc = List_Element_Index(table->flashrecordList, pListRecord, &record->id);
		if(rc)
		{
			printf("add the record to list failure");
			goto done;
		}
        flashrecord->id = id;
    } else {
     
        rc = List_Element_AddAtIndex(table->flashrecordList, record->id, (void**)&pListRecord);
		if(rc)
		{
			printf("add the record to list failure");
			goto done;
		}
    }
	record->id = flashrecord->id;
	*pListRecord = record;

	


	if(record->width <= LEVEL_0_LENGTH)
	{
		rc = Trie_Prefix_Add(table->trienode , record->prefix, LEVEL_0_LENGTH);
		if(rc)
		{
			printf("add the record from the trie failure");
			goto done;
		}	
	}else
	{


		/*if( trie_number == 2)
		{
			nhi_number = 1;
		}else if( trie_number == 4)
		{	
			nhi_number = 2;
		}else if( trie_number == 8)
		{
			nhi_number = 3;
		}else
		{	
			printf("pctrie path is error");
			rc = 1;
			goto done;
		}
		hash_key=malloc(BYTE_COUNT(record->width-nhi_number));
		memcpy(hash_key, record->prefix,record->width-nhi_number);

		for(int i = 0 ; i < nhi_number ; i++)
		{
			nhi_index = 2*nhi_index + record->prefix[record->width-nhi_number+i]-'0';
		}
	
		printf("hash_key: %s", hash_key);
		printf("nhi_index: %d", nhi_index);
		//*pctrie = hash_func(hash_key);
		//rc = pctrie_record_add(record, trie_number, nhi_index, pctrie,PCTRIE_LENGTH);
		if(rc)
		{
			printf("add the recR ord to the pctrie failure");
			goto done;
		}*/
		  
        int level = (record->width - LEVEL_0_LENGTH) / PCTRIE_LENGTH;
        int prefixlen;
        if((record->width - 16) % PCTRIE_LENGTH == 0) {
            level = level -1;
        }
        prefixlen = 16 + level * PCTRIE_LENGTH;
        memset(hash_key, 0, BYTE_COUNT(prefixlen));
        strncpy(hash_key, record->prefix , BYTE_COUNT(prefixlen));
        if((pct = hash_query(level, hash_key)) == NULL) {
			LpmModuleSet = NULL;
			rootEntryIndex = NULL;
			InitLpmModuleSet(&LpmModuleSet);
			rootId = 999;
            rc = CreatePcTrie(&pct, LpmModuleSet, rootId, -1, NULL);
            rc = hash_add(level, hash_key, pct);
            if(rc != 0) {
                printf("%s, %d: add membership failed\n", __FUNCTION__, __LINE__);
                free(pct);
                goto done;
            }
        } 
        memset(nhi_index, 0, sizeof(hash_key));
        strncpy(nhi_index, &record->prefix[prefixlen],  record->width - prefixlen);
        rc = InsertPcTrieNode(pct, pcTrieNode);;
        if(rc != 0) {
            printf("%s, %d: update pctrie failed\n", __FUNCTION__, __LINE__);
            goto done;
        }
	}
	table->recordCount += 1;
done:
	if ((rc) && (record)) {
        List_Element_DeleteAtIndex(table->flashrecordList, record->id);
        FlashRecord_Destory(record); 
    }
    return rc;
	
}
FlashError FlashTable_Record_Rm(sFlashtable* flashtable, int id )
{	
	FlashError rc  = 0;
    sFlashtable* Table   = (sFlashtable*)flashtable;
    sFlashrecord *record = NULL, ** pRecord = NULL;
	int nhi_number = 0;
	char* hash_key = NULL;
	int nhi_index  = 0;
	int hash_index = 0;
	
    rc = List_Element_Get(Table->flashrecordList, id, (void**)&pRecord);
	if(rc)
	{
		printf("get the record from list error");
	}
    record = *pRecord;
	if(record == NULL)
		goto done;
    rc = List_Element_Delete(Table->flashrecordList, pRecord);
	if(rc)
	{
		printf("del the record from list error");
	}

	if(record->width <= LEVEL_0_LENGTH)
	{
		rc = Trie_Prefix_Delete(Table->trienode, record->prefix);
		if(rc)
		{
			printf("delete the record from the trie failure");
			goto done;
		}	
	}else
	{


		/*if( trie_number == 2)
		{
			nhi_number = 1;
		}else if( trie_number == 4)
		{	
			nhi_number = 2;
		}else if( trie_number == 8)
		{
			nhi_number = 3;
		}else
		{	
			printf("pctrie path is error");
			rc = 1;
			goto done;
		}
		hash_key=malloc(BYTE_COUNT(record->width-nhi_number));
		memcpy(hash_key, record->prefix, BYTE_COUNT(record->width-nhi_number));

		for(int i = 0 ; i < nhi_number ; i++)
		{
			nhi_index = 2*nhi_index + record->prefix[record->width-nhi_number+i]-'0';
		}
		hash_index = (record->width - LEVEL_0_LENGTH)/PCTRIE_LENGTH + 1;
		printf("hash_key: %s", hash_key);
		printf("nhi_index: %d", nhi_index);
		//*pctrie = hash_func(hash_key,hash_index);
		//rc = pctrie_record_delete(record, trie_number, nhi_index, pctrie,PCTRIE_LENGTH);
		if(rc)
		{
			printf("delte the record to the pctrie failure");
			rc = 1;
			goto done;
		}*/
		
	}
	
 Table->recordCount -= 1;
done:
    return rc;
}

sFlashrecord* FlashTable_Record_Get(sFlashtable* flashtable, int id)
{
	FlashError rc = 0;
    sFlashtable* table = (sFlashtable*) flashtable;
    sFlashrecord **pRecord = NULL, *record = NULL;
    
    rc= List_Element_Get(table->flashrecordList, id, (void**)&pRecord);
	if(rc)
	{
		printf("get the record error\n");
	}
    record = *pRecord;
done:
    if (rc) return NULL;
    return (sFlashrecord*)record;
}

FlashError Dp_Lookup(sFlashtable* flashtable, int width, const char* search, sLpResult *lpresult)
{
	
	FlashError rc =0;
	int length = 0;
	int i = 0 ;
	char* hash_key = NULL;
	char* search_key =NULL;
	int hash_index = 0;
	int level_0_result = -1;
	int offset = 0;
	int result = -1;
	

	length = strlen(search);
	hash_index = (length - LEVEL_0_LENGTH)/PCTRIE_LENGTH + 1;

	rc = Trie_Prefix_Search(flashtable->trienode, search , &level_0_result);
	if(rc)
	{
		printf("in trie search error");
		goto done;
	}
	for(i = hash_index-1 ; i >= 0 ; i--)
	{
	    offset = LEVEL_0_LENGTH + i*PCTRIE_LENGTH;
		hash_key = (char *)malloc(BYTE_COUNT(offset);
		memset(hash_key, 0, BYTE_COUNT(offset);
		memcpy(hash_key,search,BYTE_COUNT(offset));
		//printf("search_key: %s", search_key);
		//*pctrie = hash(hash_key);
		/*if(pctrie != NULL)
		{
			search_key = (char *)malloc(BYTE_COUNT(length-offset));
			memeset(search , 0 , BYTE_COUNT(length-offset));
			memcpy(search_key, search+offset,length-offset);
			rc = Pctrie_Prefix_search(pctrie,search_key,&result);
			if(rc)
			{
				printf("in trie search error");
				goto done;
			}
			if(result != -1)
				 break;
				
		}*/
	}
	if (result != -1)
	{
		lpresult->match = 1;
		lpresult->level = i+2;	}else if ( level_0_result != -1)
	{
		lpresult->match = 1;
		lpresult->level = 1;
	}else
	{
		lpresult->match = 0;
		lpresult->level = 0;
	}
	
done:
	return rc;
	
}



