#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "List.h"
#include "ftrie.h"



typedef int                                 FlashError;



typedef struct s_Flashtable
{
	char*  name;
	int    id;
    int    width;
	List* flashrecordList;
	int    depth;
	int	   recordCount;
	struct s_TrieNode* trienode;
	
	

}sFlashtable;

typedef struct s_Flashrecord
{
	int    id;
	char*  prefix;
	int    width;
	sFlashtable* ftable;
}sFlashrecord;






typedef struct s_LpResult
{
	int match;
	int level; // if no match level = 0
	
}sLpResult;


FlashError FlashTable_Create(sFlashtable** flashtable);
void* FlashTable_Destory(sFlashtable* flashtable);

FlashError FlashRecord_Create(sFlashrecord** flashrecord);
void* FlashRecord_Destory(sFlashrecord* flashrecord);


FlashError FlashTable_Record_Add(sFlashtable* flashtable, sFlashrecord* flashrecord, int id);
FlashError FlashTable_Record_Rm(sFlashtable* flashtable, int id );
sFlashrecord* FlashTable_Record_Get(sFlashtable* flashtable, int id);
FlashError Dp_Lookup(sFlashtable* flashtable, int width, const char* search, sLpResult* lpresult );









