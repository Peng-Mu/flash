#ifndef __DLList_h__
#define __DLList_h__
#include "TcError.h"

typedef struct sDLList {
    struct sDLList *prev;
    struct sDLList *next;
} DLList;

typedef int (comp_func)(DLList *node1, DLList *node2);

TcError DLList_insertAfter(DLList *current, DLList *new, comp_func cmp);
TcError DLList_insertBefore(DLList *current, DLList *new, comp_func cmp);

DLList *DLList_findHead(DLList *node);
DLList *DLList_findTail(DLList *node);

#endif // __DLList_h__