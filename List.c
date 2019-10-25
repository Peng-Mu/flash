#include <stdlib.h>
#include "utils.h"
#include "TcError.h"
#include "List.h"

struct sList
{
    int   capacity;
    int   validMaskCount;
    int   eSize;
    void* validMask;
    void* buffer;
    int emptyIndex;
};

#define __ValidMask_Size(cap)       (((cap) + BITS_PER_BYTE - 1)/BITS_PER_BYTE)
#define __Buffer_Size(cap,eSize)    ((cap)*(eSize))
#define __Element_At(list, i)       ((char*)list->buffer + i*list->eSize)
#define __Index_Of(list, elm)       (((char*)elm - (char*)list->buffer) / list->eSize)
#define __Validate_Index(list, i)   \
            do {                    \
                if (0>i) {          \
                    printError(commonLibLog, "negative index. %d\n", i); \
                    rc = TcE_Invalid_Argument; goto done;                \
                }                                                        \
                else if (i>=list->capacity) {                            \
                    printError(commonLibLog, "index out-of-range. %d max=%d\n", i, list->capacity-1); \
                    rc = TcE_Out_Of_Range; goto done;                    \
                }                                                        \
            }while (0)                                                   \

#define __Validate_Index_No_Message(list, i)   \
            do {                    \
                if (0>i) {          \
                    rc = TcE_Invalid_Argument;                           \
                }                                                        \
                else if (i>=list->capacity) {                            \
                    rc = TcE_Out_Of_Range;                               \
                }                                                        \
            }while (0)                                                   \

TcError List_Create(int capacity, int eSize, List** pList) {
    TcError rc = TcE_OK;
    List*  result = NULL;
    int bufferSize = __Buffer_Size(capacity, eSize);
    int validMaskSize = __ValidMask_Size(capacity);
//    int overHeadSize = validMaskSize + sizeof(*result);
//    int percent = (overHeadSize* 100)/(bufferSize);

//    if (percent > 30) {
//        printError(commonLibLog, "overhead too large, headerSize=%X bufferSize=%X percent=%d\n", overHeadSize, bufferSize, percent);
//        return TcE_Invalid_Argument;
//    }
    result = (List*)malloc(sizeof(*result));
    if (NULL == result) {
        printError(commonLibLog, "Out-of-Memory\n");
        return TcE_Out_Of_Memory;
    }
    memset(result, 0, sizeof(*result));

    result->capacity = capacity;
    result->eSize    = eSize;

    // right after the structure header
    result->validMask = malloc(validMaskSize); 
    if (NULL == result->validMask) {
        printError(commonLibLog, "Out-of-Memory, size=%X\n", validMaskSize);
        rc = TcE_Out_Of_Memory; goto error_out;
    }
    memset(result->validMask, 0, validMaskSize);
    result->buffer = malloc(bufferSize);
    if (NULL == result->buffer) {
        printError(commonLibLog, "Out-of-Memory\n");
        rc=TcE_Out_Of_Memory; goto error_out;
    }
    *pList = result; 
    return TcE_OK;

error_out:
    if (result) {
        if (result->validMask)  free(result->validMask);
        if (result->buffer)     free(result->buffer);
        free(result);
    }
    return rc;
}

TcError List_Expand(List* list, int increase) {
    TcError rc = TcE_OK;
    int newCapacity = list->capacity + increase;
    int newValidMaskSize = __ValidMask_Size(newCapacity);
    int newBufferSize    = __Buffer_Size(newCapacity,list->eSize);
    void* newValidMask = NULL;
    void* newBuffer = NULL;

    if (increase < 0)  {
        printError(commonLibLog, "negagive increase not supported %d\n", increase);
        rc=TcE_Invalid_Argument; goto done;
    }
    newValidMask = realloc(list->validMask, newValidMaskSize+newBufferSize);
    if (NULL == newValidMask) {
        printError(commonLibLog, "Out-of-Memory\n");
        rc = TcE_Out_Of_Memory; goto done;
    }
    newBuffer = realloc(list->buffer, newBufferSize);
    if (NULL == newBuffer) {
        printError(commonLibLog, "Out-of-Memory\n");
        rc = TcE_Out_Of_Memory; goto done;
    }
    list->validMask = newValidMask;
    memset((char*)list->validMask + __ValidMask_Size(list->capacity), 
                  0,
                  (newValidMaskSize - __ValidMask_Size(list->capacity)));

    list->buffer    = newBuffer;
    list->capacity  += increase;

    return TcE_OK;
done:
    if (newValidMask) 
        list->validMask = realloc(newValidMask, __ValidMask_Size(list->capacity));
    if (newBuffer)
        list->buffer = realloc(newBuffer, __Buffer_Size(list->capacity,list->eSize)); 
    return rc;
}

void List_Destroy(List* list)
{
    if (NULL == list)
        return;

    free(list->validMask);
    free(list->buffer);
    free(list);
}

static int __List_Clear_ValidMask_At(List* list, int index) {
    int byte = index / BITS_PER_BYTE;
    int bit  = index % BITS_PER_BYTE;
    char* p = (char*)list->validMask + byte;
    
    if (0==(p[0] & (1<<(bit)))) {
        printError(commonLibLog, "index already clear. %X\n", index);
        return TcE_Invalid_Argument;
    }
    p[0] &= ~(1<<bit);
    list->validMaskCount--;
    return TcE_OK;
}

static int __List_Set_ValidMask_At(List* list, int index) {
    TcError rc = TcE_OK;
    int byte = index / BITS_PER_BYTE;
    int bit  = index % BITS_PER_BYTE;
    char* p = (char*)list->validMask + byte;
    
    assertLog(index>=0);
    assertLog(index<list->capacity);
    if (p[0] & (1<<(bit))) {
        printError(commonLibLog, "index already taken. %X\n", index);
        return TcE_Invalid_Argument;
    }
    p[0] |= (1<<bit);
    list->validMaskCount++;
done:
    return rc;
}

static int __List_IsSet_ValidMask_At(List* list, int index) {
    TcError rc = TcE_OK;
    int byte=index/BITS_PER_BYTE;
    int bit=index%BITS_PER_BYTE;
    char* p=(char*)list->validMask;

    assertLog(index>=0);
    assertLog(index<list->capacity);

    return (p[byte] & (1<<bit));
done:
    return rc;
}

static int __List_IsClear_ValidMask_At(List* list, int index) {
    return !__List_IsSet_ValidMask_At(list, index);
}

static int  __List_Get_Next_FreeIndex(List *list, int index){	
    TcError rc = TcE_OK;
	int i,j;
    char* p;
    int n=list->capacity/BITS_PER_BYTE;

    assertLog(list);
    p=(char*)list->validMask;
    for (i=index/BITS_PER_BYTE;i<n;i++) {
        if (p[i] != (char)0xFF) break;
    }
    if (i==n) n=list->capacity % BITS_PER_BYTE;
    else      n=BITS_PER_BYTE;
    for (j=0; j<n; j++) {
       if (0==(p[i] & (1<<j))) break;    
    }
    return i*BITS_PER_BYTE + j;
done:
    return rc;
}

static int __List_GetAndSet_FreeIndex(List* list) {
    TcError rc = TcE_OK;
    int newEmptyIndex = -1;
    int returnIndex = list->emptyIndex;
    CHECK_ERROR(commonLibLog, __List_Set_ValidMask_At(list, list->emptyIndex));
    newEmptyIndex = __List_Get_Next_FreeIndex(list,list->emptyIndex);
    list->emptyIndex = newEmptyIndex;
    return returnIndex;
done:
   return rc;
}

static int __List_First_Valid_Index(List* list, int start, int* pIndex) {
    TcError rc = TcE_OK;
    int i,j;
    char* p=(char*)list->validMask;
    int startByte = start / BITS_PER_BYTE;
    int startBit = start % BITS_PER_BYTE;
    int endByte = list->capacity / BITS_PER_BYTE;
    int endBit  = list->capacity % BITS_PER_BYTE;

    assertLog(list);
    assertLog(start>=0);
    assertLog(start<=list->capacity);
    i=startByte;
    // start and capacity on the same byte
    if (startByte == endByte) {
        for (j=startBit; j<endBit; j++) {
           if (p[i] & (1<<j)) { 
               *pIndex = i*BITS_PER_BYTE + j;
               return 1;
           }
        }
    } else {
        // scan partial byte at begin if exist
        if (startBit) {
            for (j=startBit; j<BITS_PER_BYTE; j++) {
                if (p[i] & (1<<j)) {
                    *pIndex = i*BITS_PER_BYTE + j;
                    return 1;
                }
            }        
            i++;
        }
        for (; i<endByte; i++) {
            if (p[i]) {
                for (j=0; j<BITS_PER_BYTE; j++) {
                    if (p[i]&(1<<j)) {
                        *pIndex = i*BITS_PER_BYTE + j;
                        return 1;
                    }
                }
            }
        }
        if (endBit) {
            for (j=0; j<endBit; j++) {
                if (p[i]&(1<<j)) {
                    *pIndex = i*BITS_PER_BYTE + j;
                    return 1;
                }
            }
        }
    }
    return 0;
done:
    return rc;
}

TcError List_Element_Add(List* list, void** element) {
    int index;
    if (list->validMaskCount == list->capacity) {
        printError(commonLibLog, "No more space left\n");
        return TcE_Capacity_Reached;
    }
    index = __List_GetAndSet_FreeIndex(list);
    if (index < 0) return index; 
    *element = __Element_At(list, index);
    return TcE_OK;
}

TcError List_Element_AddAtIndex(List* list, int id, void** element) {
    int rc = TcE_OK;
    
    if (list->validMaskCount == list->capacity) {
        printError(commonLibLog, "No more space left\n");
        return TcE_Capacity_Reached;
    }
    __Validate_Index(list, id);
    CHECK_ERROR(commonLibLog, __List_Set_ValidMask_At(list, id));
    if (id == list->emptyIndex) {
        list->emptyIndex = __List_Get_Next_FreeIndex(list,list->emptyIndex);
    }
    *element = __Element_At(list, id);
done:
    return rc;
}

TcError List_Element_Delete(List* list, void* element) {
    TcError rc = TcE_OK;
    int index;

    CHECK_ERROR(commonLibLog, 
            List_Element_Validate(list, element, &index));

    __List_Clear_ValidMask_At(list, index);
    if (index < list->emptyIndex)   list->emptyIndex = index;

done:
    return rc;
}

int List_Element_Count(List* list) {
    return list->validMaskCount;
}

TcError List_Element_Get(List* list, int index, void** element) {
    TcError rc = TcE_OK;
    rc = __List_IsSet_ValidMask_At(list, index);
    if (rc<0) goto done;
    if (0==rc) {
        printError(commonLibLog, "Empty slot at %d\n", index);
        return TcE_Invalid_Argument;
    }
    if (element) {
        *element = __Element_At(list, index);
    }
    return TcE_OK;
done:
    return rc;
}

TcError List_Element_Validate(List* list, const void* element, int* pIndex) {
    TcError rc = TcE_OK;
    size_t displacement = ((char*)element - (char*)list->buffer);
    int  i;
    if (element < list->buffer) {
        printError(commonLibLog, "element point to some point outside of storage buffer\n");
        return TcE_Invalid_Argument;
    }
    if (displacement % list->eSize) {
        printError(commonLibLog, "misaligned element pointer\n");
        return TcE_Invalid_Argument;
    }
    i = displacement / list->eSize;
    if (i>=list->capacity) {
        printError(commonLibLog, "element not found. %d\n", i);
        return TcE_Invalid_Argument;
    }
    rc = __List_IsSet_ValidMask_At(list, i);
    if (rc<0) goto done;
    if (0==rc) {
        printError(commonLibLog, "valid mask not set at. %d\n", i);
        return TcE_Invalid_Argument;
    }
    if (pIndex) { *pIndex = i; }
    return TcE_OK;
done:
    return rc;
}

int List_Capacity(List* list) {
    return list->capacity;
}

//void List_Consolidate(List* list)
//{
//    int copy = 0;
//    int scan = 0;
//    int count;
//    int i;
//    void* dst;
//    void* src;
//
//    if (!list->freesCount)
//        return;
//
//    // find empty slot index
//    i=0;
//    while (i<list->capacity)
//    {
//        if (list->valids[i])   { i++; }
//        else                   { copy = i; break;  }
//    }
//
//    while (i<list->capacity)
//    {
//        // find valid slot
//        while (i<list->capacity)
//        {
//            if (!list->valids[i])   { i++; }
//            else                    { scan = i; break;  }
//        }
//
//        if (i == list->capacity)
//            break;
//
//        // count valid slots
//        count = 0;
//        while (i<list->capacity)
//        {
//            if (list->valids[i])   { i++; count++;}
//            else                   { break; }
//        }
//
//        dst = (char*)list->buffer + copy * list->eSize;
//        src = (char*)list->buffer + scan * list->eSize;
//        memcpy(dst, src, count*list->eSize);
//        copy += count;
//    }
//}

void* List_Buffer(List* list) {
    return list->buffer;
}

int List_Element_Index(List* list, const void* element, int* pIndex) {
    TcError rc = TcE_OK;

    CHECK_ERROR(commonLibLog,
        List_Element_Validate(list, element, pIndex));    

done:
    return rc;
}

//TcError List_Element_Reserve(List* list, int index, void** element) {
//    TcError rc = TcE_OK;
//    rc = __List_IsSet_ValidMask_At(list, index);
//    if (rc<0) goto done;
//    if (rc) {
//        printError(commonLibLog, "the slot has been taken %X\n", index);
//        return TcE_Invalid_Argument;
//    }
//    CHECK_ERROR(commonLibLog, __List_Set_ValidMask_At(list, index));
//    *element = __Element_At(list, index);
//    if (index == list->emptyIndex){
//		list->emptyIndex = __List_Get_Next_FreeIndex(list,list->emptyIndex);
//	} 
//    return TcE_OK;
//done:
//   return rc;
//}

TcError List_Element_Release(List* list, void* element) {
    TcError rc = TcE_OK;
    int index;
   
    CHECK_ERROR(commonLibLog, 
            List_Element_Index(list, element, &index));
    __List_Clear_ValidMask_At(list, index);
    if (index < list->emptyIndex) list->emptyIndex = index;
done:
    return rc;
}

void* List_Element_First(List* list) {
    int index; 
    void* elm = NULL;

    if (list->validMaskCount== 0) return NULL;
    if (__List_First_Valid_Index(list,0,&index)) { elm= __Element_At(list, index); }
    return elm;
}

void* List_Element_Next(List* list, const void* prev) {
    void* next = NULL;
    int index;
    TcError rc = TcE_OK;
   
    CHECK_ERROR(commonLibLog,
          List_Element_Index(list, prev, &index));
    
    if (__List_First_Valid_Index(list, index+1, &index)) {
        next = __Element_At(list, index);
    }

done:
    return next;
}

TcError List_Element_AddAtSmallestAvailIndex(List* list, void** element) {
    TcError rc = TcE_OK;

    CHECK_ERROR(commonLibLog,
        List_Element_Add(list, element));

done:
    return rc;
}

int List_IsEmpty_At(List* list, int index) {
    if (0>index)                { return 0; }
    else if (index>=list->capacity) { return 0; }                                                        
    return __List_IsClear_ValidMask_At(list, index); 
}

TcError List_Element_DeleteAtIndex(List* list, int index)
{
    TcError rc = TcE_OK;
    __Validate_Index(list, index);

    rc = __List_IsClear_ValidMask_At(list, index);
    if (rc < 0) goto done;
    if (rc) {
        printError(commonLibLog, "slot empty at index %d\n", index);
        rc = TcE_Invalid_Argument; goto done;
    }
    __List_Clear_ValidMask_At(list, index);
    if (index < list->emptyIndex)  list->emptyIndex = index;

done:
    return rc;
}

int List_IsValid_At(List* list, int index)
{
    if (0>index)                { return 0; }
    else if (index>=list->capacity) { return 0; }                                                        
    return __List_IsSet_ValidMask_At(list, index);
}




