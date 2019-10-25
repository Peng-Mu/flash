#ifndef __List_h__
#define __List_h__

#include "TcError.h"
#include "Platform.h"

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct sList List;

API TcError List_Create(int capacity, int eSize, List** list);
API TcError List_Expand(List* list, int increase);
API void List_Destroy(List* list);

API TcError List_Element_Add(List* list, void** element);
API TcError List_Element_AddAtIndex(List* list, int i, void** element);
API TcError List_Element_AddAtSmallestAvailIndex(List* list, void** element);
API TcError List_Element_Delete(List* list, void* element);
API TcError List_Element_DeleteAtIndex(List* list, int index);
API TcError List_Element_Validate(List* list, const void* element, int* pIndex);
API int     List_Element_Count(List* list);
API TcError List_Element_Get(List* list, int index, void** element);
API TcError List_Element_Index(List* list, const void* element, int* pIndex);
//API TcError List_Element_Reserve(List* list, int index, void** element);
API TcError List_Element_Release(List* list, void* element);
API int List_IsEmpty_At(List* list, int index);
API int List_IsValid_At(List* list, int index);

API void* List_Element_First(List* list);
API void* List_Element_Next(List* list, const void* prev);

API int List_Capacity(List* list);
//API void List_Consolidate(List* list);
//API void* List_Buffer(List* list);

#ifdef __cplusplus
}
#endif

#endif // __List_h__
