#include <stdlib.h>
#include "DLList.h"
#include "log.h"

DLList *DLList_findHead(DLList *node) {
    if (!node) return node;
    while (node->prev) {
        node = node->prev;
    }
    return node;
}

DLList *DLList_findTail(DLList *node) {
    if (!node) return node;
    while (node->next) {
        node = node->next;
    }
    return node;
}

TcError DLList_insertAfter(DLList *current, DLList *new, comp_func comp) {
    TcError rc = TcE_OK;
    assertLog(comp(current, new)>0);
    while (current) {
        if (comp(current, new)<=0) {
            new->prev = current->prev;
            new->next = current;
            current->prev = new;
            if (new->prev) new->prev->next = new;
            break;
        }
        if (current->next==NULL) {
            new->next = NULL;
            new->prev = current;
            current->next = new;
            break;
        }
        current = current->next;
    }
done:
    return rc;
}

TcError DLList_insertBefore(DLList *current, DLList *new, comp_func comp) {
    TcError rc = TcE_OK;
    assertLog(comp(current, new)<=0);
    while (current) {
        if (comp(current, new)>0) {
            new->next = current->next;
            new->prev = current;
            current->next = new;
            if (new->next)  new->next->prev = new;
            break;
        }
        if (current->prev==NULL) {
            new->prev = NULL;
            new->next = current;
            current->prev = new;
            break;
        }
        current = current->prev;
    }
done:
    return rc;
}


