/*****MARVELL STARGATE NSE********************************************************************/
/*****Application Example: stargate0 *********************************************************
Add one table
Add one record before initial allocation
Start allocation
Search on this record
Add one record during runtime
allocation during runtime
search on this record
**********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "nse.h"
#include "npu_platform.h"
#include "pc_trie.h"

#define LOG_NAME "nseApp0.log"

void str2binary(char* str, int width);
void binary2str(char* in, char* out, int width);
void displayResults(int* readys, int* matchs, int* priorities);

int main(int argc, const char* argv[])
{
	NHIModuleSet* LpmModuleSet = NULL;
	NHITableEntry* rootEntryIndex = NULL;
	InitLpmModuleSet(&LpmModuleSet);
	pc_Trie *pcTrie = NULL;
	int rootId = 999;
	CreatePcTrie(&pcTrie, LpmModuleSet, rootId, -1, NULL);
	pc_TrieNode *pcTrieNode = malloc(sizeof(pc_TrieNode));
	pcTrieNode->lpmRecordId = 0;
	pcTrieNode->lenth = 2;
	memset(pcTrieNode->recordData, 0, sizeof(pcTrieNode->recordData));
	pcTrieNode->recordData[0] = 0x02;//10100
	InsertPcTrieNode(pcTrie, pcTrieNode);
	pcTrieNode->lpmRecordId = 1;
	pcTrieNode->lenth = 5;
	memset(pcTrieNode->recordData, 0, sizeof(pcTrieNode->recordData));
	pcTrieNode->recordData[0] = 0x14;//10100
	InsertPcTrieNode(pcTrie, pcTrieNode);
	pcTrieNode->lpmRecordId = 2;
	pcTrieNode->lenth = 2;
	memset(pcTrieNode->recordData, 0, sizeof(pcTrieNode->recordData));
	pcTrieNode->recordData[0] = 0x01;//10100
	InsertPcTrieNode(pcTrie, pcTrieNode);
	return 0;
}


void str2binary(char* str, int width)
{
    int i, j;
    for (i=0; i<width/8; i++)
    {
        char byte = 0;
        for (j=0; j<8; j++)
        {
            int idx = 8*i + j;
            char value = (idx < width) 
                            ? str[idx] - '0' 
                            : 0;
            byte = byte<<1 | value;
        }
        str[i] = byte;
    }
    str[i] = '\0';
}

void binary2str(char* in, char* out, int width)
{
    int i, j;
    char buff[1024] = "";
    memcpy(buff, in, width);
    for (i=0; i<width; i++)
    {
        for (j=7; j>=0; j--)
        {
            out[8*i+j] = '0' + ((int)buff[i]&0x01);
            buff[i] = buff[i]>>1;
        }
        out[width*8] = 0;
    }
}

void displayResults(int* readys, int* matchs, int* priorities)
{
    int i;
    printf("Channle    Ready    Match     Priority\n");
    for (i=0; i<4; i++)
    {
        printf("%7d    %5d    %5d     %d\n", i, readys[i], matchs[i], priorities[i]);
    }
}

