#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interface.h"

int main(int argc, const char* argv[])
{
	
	sFlashtable *flashtable = NULL;
	sFlashrecord *flashrecord = NULL;
	sLpResult *lpresult = (sLpResult *)malloc(sizeof(lpresult));
	lpresult->level = 0 ;
	lpresult->match = 0;
	//sFlashrecord *flash = NULL;
	int rc = 0;
	int num, last, id, i,leng;
	char *search;
    char p[144]="";
	FILE *f;
	f = fopen(argv[1], "r");
    if(!f) {
        printf("file open failed\n");
        return 0;
    }
		
//	int i, j = 1;
//	for(i = 0; i < 14; i++)
//	{
//		while ((j >= pow(2,i)) && (j < pow(2,i+1)))
//		{
//			printf("%d(%d) ", j, root[j].id);
//			j++;
//		}
//		printf("\n");
//	}
	
	rc = FlashTable_Create(&flashtable);
	if(rc != 0)
	{
		printf("create flashtable failure");
		goto done;
	}
	flashtable->depth = 2*1024*1024;
	flashtable->id 	  = 0;
	flashtable->name  = "T1";
	flashtable->recordCount = 0;
	flashtable->width = 144;
	
	rc = FlashRecord_Create(&flashrecord);
	if( rc != 0)
	{
		printf("create flashrecord failure");
		goto done;
	}
	
	i = 0 ;
	while(fgets(p, 144, f) != NULL)
	{
		leng = strlen(p)-2;
		if(leng == 0 && p[0] == '*')
		{
			continue;
			i++;
		}
		//printf("rules: %s \n",  p );
		flashrecord->ftable = flashtable;
		flashrecord->id = i;
		flashrecord->width = leng;
		flashrecord->prefix = malloc(leng+1);
		memcpy(flashrecord->prefix,p,leng);
		flashrecord->prefix[leng] = '\0';
		//printf("rules: %s  \n",  flashrecord->prefix );
		//printf("length: %d \n", flashrecord->width);
		rc = FlashTable_Record_Add(flashtable, flashrecord, i);
		if( rc != 0)
		{
			printf("add flashrecord failure");
			goto done;
		}
		i++;
		memset(p, 0, leng+1);
		if(i%10240 == 0)
			printf("--------%d----------\n", i);
	 }
	
	num = i;
	/*memset(flashrecord, 0, sizeof(*flashrecord));
	for( i = 0 ;  i < num; i++ )
	{
		flashrecord = FlashTable_Record_Get(flashtable, i);
		if(flashrecord == NULL)
		{
			printf("get flashrecord failure");
			goto done;
		}
		printf("%d th rules: %s \n", i, flashrecord->prefix );
		rc = Dp_Lookup(flashtable, flashrecord->width, flashrecord->prefix, lpresult);
		if(rc!=0)
		{
			printf("search failure");
			goto done;	
		}
		printf("match : %d , level: %d \n", lpresult->match,lpresult->level);
	}*/
	

	/*for( i = 0 ;  i < 4; i++ )
	{
		flashrecord = FlashTable_Record_Get(flashtable, i);
		if(flashrecord == NULL)
		{
			printf("get flashrecord failure");
			goto done;
		}
		search= flashrecord->prefix ;
		//printf("rules: %s \n",  search );
		rc = FlashTable_Record_Rm(flashtable, i);
		if(flashrecord == NULL)
		{
			printf("rm flashrecord failure");
			goto done;
		}
		rc = Dp_Lookup(flashtable, flashrecord->width, search, lpresult);
		if(rc!=0)
		{
			printf("search failure");
			goto done;	
		}
		printf("match : %d , level: %d \n", lpresult->match,lpresult->level);
	}*/
	hash_status();
	free(lpresult);
	
	
done:
	//if (flashrecord) FlashRecord_Destory(flashrecord);
	//if (flashtable) FlashTable_Destory(flashtable);
    
	return rc;
}
