/*
 * Copyright (C) 2007 - 2013 Campanoni Simone, Luca Rocchini, Michele Tartara, Timothy M. Jones
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <platform_API.h>

/* My headers	*/
#include <xanlib.h>
#include <xan-system.h>
#include <config.h>

static inline void internal_freeItem (XanList *l, XanListItem *item);
static inline XanListItem * internal_allocateItem (XanList *l);
static inline void internal_destroyBuffer (XanList *list);

/**
 * Unsynchronised functions.
 **/

XanList * xanList_new (void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void * (*cloneFunction)(void *addr)) {
    XanList *list;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        list		= malloc(sizeof(XanList));
        list->alloc	= malloc;
    } else {
        list		= allocFunction(sizeof(XanList));
        list->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        list->free		= free;
    } else {
        list->free		= freeFunction;
    }

    /* Make the list	*/
    assert(list != NULL);
    list->firstItem = NULL;
    list->lastItem = NULL;
    list->len = 0;
    list->bufferItems   = NULL;
    list->bufferItemsTop    = 0;
    list->bufferItemsSize   = 0;
    list->clone = cloneFunction;
    pthread_mutexattr_t mutex_attr;
    PLATFORM_initMutexAttr(&mutex_attr);
    PLATFORM_setMutexAttr_type(&mutex_attr, PTHREAD_MUTEX_ADAPTIVE_NP);
#ifdef MULTIAPP
    PLATFORM_setMutexAttr_pshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
#endif
    PLATFORM_initMutex(&(list->itemsMutex), &mutex_attr);

    /* Return		*/
    return list;
}

XanHashTable * xanList_toHashTable (XanList *l) {
    XanHashTable	*t;
    XanListItem	*item;

    /* Assertions.
     */
    assert(l != NULL);

    /* Allocate a new table.
     */
    t	= xanHashTable_new(11, 0, l->alloc, realloc, l->free, NULL, NULL);

    item	= xanList_first(l);
    while (item != NULL) {
        xanHashTable_insert(t, item->data, item->data);
        item	= item->next;
    }

    return t;
}

void * xanList_getData (XanListItem *item) {

    /* Assertions		*/
    assert(item != NULL);

    return item->data;
}

XanListItem *xanList_first (XanList *list) {

    /* Assertions		*/
    assert(list != NULL);

    /* Return               */
    return list->firstItem;
}

XanListItem * xanList_last (XanList *list) {

    /* Assertion            */
    assert(list != NULL);

    /* Return               */
    return list->lastItem;
}

XanListItem * xanList_next (XanListItem *item) {

    /* Assertions		*/
    assert(item != NULL);

    return item->next;
}

XanListItem * xanList_prev (XanListItem *item) {

    /* Assertions		*/
    assert(item != NULL);

    return item->prev;
}

XanListItem * xanList_append (XanList *list, void *data) {
    XanListItem     *item;
    XanListItem     *itemToAppend;

    /* Assertions.
     */
    assert(list != NULL);

    /* Initialize the variables.
     */
    item 		= NULL;
    itemToAppend 	= NULL;

    /* Make a new item.
     */
    itemToAppend 		= internal_allocateItem(list);
    assert(itemToAppend != NULL);
    itemToAppend->data 	= data;
    itemToAppend->next 	= NULL;

    /* Append the new item.
     */
    if (list->firstItem == NULL) {
        assert(list->len == 0);
        list->firstItem = itemToAppend;
        itemToAppend->prev = NULL;
    } else {
        assert(list->len > 0);
        assert(list->lastItem != NULL);
        item = list->lastItem;
        item->next = itemToAppend;
        itemToAppend->prev = item;
    }
    (list->len)++;
    list->lastItem = itemToAppend;
    assert(list->len > 0);

    return itemToAppend;
}

void xanList_appendList (XanList *list, XanList *listToAppend) {
    XanListItem     *item;
    void            *data;

    /* Assertions.
     */
    assert(listToAppend != NULL);
    assert(list != NULL);

    /* Append the list.
     */
    item = xanList_first(listToAppend);
    while (item != NULL) {
        data = item->data;
        xanList_append(list, data);
        item = item->next;
    }

    return ;
}

XanListItem * xanList_insert (XanList *list, void *data) {
    XanListItem     *item;
    XanListItem     *itemToInsert;

    /* Assertions			*/
    assert(list != NULL);

    /* Initialize the variables	*/
    item = NULL;
    itemToInsert = NULL;

    /* Make a new item		*/
    itemToInsert = internal_allocateItem(list);
    assert(itemToInsert != NULL);
    itemToInsert->data = data;
    itemToInsert->prev = NULL;

    /* Insert the new item		*/
    if (list->firstItem == NULL) {
        assert(list->len == 0);
        list->firstItem = itemToInsert;
        list->lastItem = itemToInsert;
        itemToInsert->next = NULL;
    } else {
        assert(list->len > 0);
        item = list->firstItem;
        assert(item != NULL);
        list->firstItem = itemToInsert;
        itemToInsert->next = item;
        item->prev = list->firstItem;
    }
    (list->len)++;
    assert(list->len > 0);
    assert(list->lastItem != NULL);

    return itemToInsert;
}

XanListItem * xanList_insertBefore (XanList *list, XanListItem *prev, void *data) {
    XanListItem     *item;
    XanListItem     *itemToAppend;

    /* Assertions			*/
    assert(list != NULL);
    assert(prev != NULL);
    assert(list->firstItem != NULL);

    /* Initialize the variables.
     */
    item = NULL;
    itemToAppend = NULL;

    /* Check the trivial cases.
     */
    if (list->firstItem == prev) {
        return xanList_insert(list, data);
    }

    /* Make a new item.
     */
    itemToAppend = internal_allocateItem(list);
    assert(itemToAppend != NULL);
    itemToAppend->data = data;
    itemToAppend->next = NULL;

    /* Insert the new item		*/
    item = prev->prev;
    item->next = itemToAppend;
    prev->prev = itemToAppend;
    itemToAppend->prev = item;
    itemToAppend->next = prev;
    (list->len)++;
    assert(list->lastItem != NULL);

    return itemToAppend;
}

XanListItem * xanList_insertAfter (XanList *list, XanListItem *prev, void *data) {
    XanListItem     *item;
    XanListItem     *itemToAppend;

    /* Assertions.
     */
    assert(list != NULL);
    assert(prev != NULL);

    /* Initialize the variables.
     */
    item = NULL;
    itemToAppend = NULL;

    /* Check whether we can append the new item.
     */
    if ((list->len == 1) || (list->len == 0)) {
        return xanList_append(list, data);
    }
    assert(list->len > 1);

    /* Make a new item.
     */
    itemToAppend = internal_allocateItem(list);
    assert(itemToAppend != NULL);
    itemToAppend->data = data;
    itemToAppend->next = NULL;
    itemToAppend->prev = NULL;

    item = prev->next;
    prev->next = itemToAppend;
    itemToAppend->prev = prev;
    itemToAppend->next = item;
    if (item != NULL) {
        assert(itemToAppend->next != NULL);
        item->prev = itemToAppend;
    }
    (list->len)++;

    if (itemToAppend->next == NULL) {
        list->lastItem = itemToAppend;
    }
    assert(list->lastItem != NULL);

    return itemToAppend;
}

void xanList_moveBeforeItem (XanList *list, XanListItem *itemToMove, XanListItem *place) {
    XanListItem	*placePrev;
    XanListItem	*itemToMovePrev;
    XanListItem	*itemToMoveNext;

    /* Assertions.
     */
    assert(list != NULL);
    assert(itemToMove != NULL);
    assert(place != NULL);
    assert(list->firstItem->prev == NULL);
    assert(list->lastItem->next == NULL);

    /* Fetch the surrounding items.
     */
    placePrev		= place->prev;
    itemToMovePrev		= itemToMove->prev;
    itemToMoveNext		= itemToMove->next;

    /* Check if we need to do anything.
     */
    if (placePrev == itemToMove) {

        /* ItemToMove is already just before place.
         * Hence, nothing need to be done.
         */
        return ;
    }

    /* Adjust the first element.
     */
    if (list->firstItem == itemToMove) {
        list->firstItem	= itemToMoveNext;
    }

    /* Place the item to move just before place.
     * Attach the item to move as the successor of placePrev.
     */
    itemToMove->prev	= placePrev;
    if (placePrev != NULL) {
        placePrev->next		= itemToMove;
    }

    /* Attach the item to move as the predecessor of place.
     */
    itemToMove->next	= place;
    place->prev		= itemToMove;

    /* Remove the gap left by moving itemToMove.
     */
    if (itemToMovePrev != NULL) {
        itemToMovePrev->next	= itemToMoveNext;
    }
    if (itemToMoveNext != NULL) {
        itemToMoveNext->prev	= itemToMovePrev;
    }

    /* Fix the first and last element.
     */
    if (list->firstItem == place) {
        list->firstItem	= itemToMove;
    }
    if (list->lastItem == itemToMove) {
        list->lastItem	= itemToMovePrev;
        assert(list->lastItem != NULL);
    }
    assert(list->firstItem->prev == NULL);
    assert(list->lastItem->next == NULL);

    return ;
}

XanListItem * xanList_moveToBegin (XanList *list, XanListItem *item) {
    XanListItem     *itemTemp;

    /* Assertions			*/
    assert(list != NULL);
    assert(item != NULL);
    assert(xanList_length(list) > 0);
    assert(list->len > 0);
#ifdef DEBUG
    void    *element;
    element = item->data;
    assert(element != NULL);
    assert(xanList_find(list, element) != NULL);
#endif

    /* Move the item to the begin   */
    if (xanList_first(list) != item) {
        assert(list->len > 0);
        assert(list->firstItem->prev == NULL);
        assert(item->prev != NULL);
        assert(list->lastItem != NULL);

        /* Update the lastItem field of the list        */
        if (list->lastItem == item) {
            list->lastItem = item->prev;
        }
        assert(list->lastItem != NULL);

        itemTemp = list->firstItem;
        assert(itemTemp != NULL);
        assert(itemTemp->prev == NULL);
        item->prev->next = item->next;
        assert(item->prev->next != item);
        if (item->next != NULL) {
            item->next->prev = item->prev;
            assert(item->next->prev != item);
        }
        list->firstItem = item;
        item->next = itemTemp;
        item->prev = NULL;
        itemTemp->prev = list->firstItem;
        assert(itemTemp != item);
        assert(list->firstItem == item);
        assert(list->firstItem->next == itemTemp);
        assert(list->firstItem->prev == NULL);
        assert(itemTemp->prev == list->firstItem);
    }
    assert(list->firstItem != NULL);
    assert(list->firstItem == item);
    assert(list->firstItem->prev == NULL);

    return list->firstItem;
}

XanListItem * xanList_find (XanList *list, void *data) {
    XanListItem     *item;
    XanListItem     *champion;
    void            *current_data;

    /* Assertions		*/
    assert(list != NULL);

    champion = NULL;
    current_data = NULL;
    item = xanList_first(list);
    while (item != NULL) {
        current_data = item->data;
        if (current_data == data) {
            champion = item;
            break;
        }
        item = item->next;
    }

    return champion;
}

int xanList_length (XanList *list) {

    /* Assertions.
     */
    assert(list != NULL);

    return list->len;
}

int xanList_getPositionNumberFromElement (XanList *list, void *data) {
    int count;
    XanListItem     *item;

    /* Assertions			*/
    assert(list != NULL);

    /* Find the data		*/
    count = 0;
    item = xanList_first(list);
    while (item != NULL) {
        if (item->data == data) {
            return count;
        }
        count++;
        item = item->next;
    }

    /* The element was not found.	*/
    return -1;
}

XanListItem * xanList_getElementFromPositionNumber (XanList *list, int positionNumber) {
    XanListItem     *item;
    int count;

    /* Assertions			*/
    assert(list != NULL);

    count = 0;
    if (positionNumber >= list->len) {
        return NULL;
    }
    if (positionNumber == 0) {
        return list->firstItem;
    }
    if (positionNumber == (list->len - 1)) {
        return list->lastItem;
    }

    item = xanList_first(list);
    while (item != NULL) {
        if (count == positionNumber) {
            return item;
        }
        count++;
        item = item->next;
    }
    fprintf(stderr, "XANLIB: ERROR = The element in the position %d does not exist in the list and it should be present.\n", positionNumber);
    abort();
}

bool xanList_shareSomeElements (XanList *list, XanList *otherList) {
    void            *data;
    void            *data2;
    XanListItem     *item;
    XanListItem     *item2;

    /* Assertions		*/
    assert(list != NULL);
    assert(otherList != NULL);

    item = xanList_first(list);
    while (item != NULL) {
        data = item->data;
        item2 = xanList_first(otherList);
        while (item2 != NULL) {
            data2 = item2->data;
            if (data == data2) {
                return true;
            }
            item2 = item2->next;
        }
        item = item->next;
    }

    return false;
}

bool xanList_containsTheSameElements (XanList *list, XanList *otherList) {
    XanListItem     *item;

    /* Assertions			*/
    assert(list != NULL);
    assert(otherList != NULL);

    /* Check the length		*/
    if (list->len != otherList->len) {
        return false;
    }

    /* Check the list		*/
    item = xanList_first(list);
    while (item != NULL) {
        if (xanList_find(otherList, item->data) == NULL) {
            return false;
        }
        item = item->next;
    }

    /* The two lists contain the    *
    * same set of elements.	*
    * Return                       */
    return true;
}

bool xanList_containsAllElements (XanList *subList, XanList *superList) {
    XanListItem     *item;

    /* Assertions			*/
    assert(subList != NULL);
    assert(superList != NULL);

    /* Check the list		*/
    item = xanList_first(subList);
    while (item != NULL) {
        if (xanList_find(superList, item->data) == NULL) {
            return false;
        }
        item = item->next;
    }

    /* The second list contains    *
    * elements in the first list.  *
    * Return                       */
    return true;
}

int xanList_equalsInstancesNumber (XanList *list, void *data) {
    XanListItem     *item;
    void            *tempData;
    int num;

    /* Assertions			*/
    assert(list != NULL);

    num = 0;
    item = xanList_first(list);
    while (item != NULL) {
        tempData = item->data;
        if (tempData == data) {
            num++;
        }
        item = item->next;
    }

    /* Return			*/
    return num;
}

void ** xanList_getSlotData (XanListItem *item) {

    /* Assertions		*/
    assert(item != NULL);

    return &(item->data);
}

XanList * xanList_getSharedElements(XanList *list1, XanList *list2) {
    XanList *shared;
    XanListItem *item1;

    /* Assertions.
     */
    assert(list1);
    assert(list2);

    shared = xanList_new(list1->alloc, list1->free, list1->clone);
    item1 = xanList_first(list1);
    while (item1) {
        if (xanList_find(list2, item1->data)) {
            xanList_append(shared, item1->data);
        }
        item1 = item1->next;
    }

    return shared;
}

void xanList_emptyOutList (XanList *list) {
    XanListItem     *item;

    /* Assertions.
     */
    assert(list != NULL);

    /* Flush out the list.
     */
    item = xanList_first(list);
    while (item != NULL) {
        xanList_deleteItem(list, item);
        item = xanList_first(list);
    }
    assert(list->firstItem == NULL);
    assert(list->lastItem == NULL);

    return;
}

void xanList_destroyList (XanList *list) {
    XanListItem     *item;
    XanListItem     *itemprev;

    /* Assertions.
     */
    assert(list != NULL);

    /* Free the list elements.
     */
    item = xanList_first(list);
    while (item != NULL) {
        itemprev = item;
        item = item->next;
        list->free(itemprev);
    }

    /* Free the buffer.
     */
    internal_destroyBuffer(list);

    /* Free the list.
     */
    list->free(list);

    return ;
}

void xanList_destroyListAndData (XanList *list) {
    XanListItem     *item;
    XanListItem     *itemprev;

    /* Assertions.
     */
    assert(list != NULL);

    /* Free the list elements.
     */
    item = xanList_first(list);
    while (item != NULL) {
        itemprev = item;
        item = item->next;
        list->free(itemprev->data);
        list->free(itemprev);
    } 

    /* Free the buffer.
     */
    internal_destroyBuffer(list);

    /* Free the list.
     */
    list->free(list);

    return ;
}

void xanList_deleteItem (XanList *list, XanListItem *item) {
    XanListItem    	 	*prevItem;
    XanListItem    	 	*nextItem;

    /* Assertions.
     */
    assert(list != NULL);
    assert(item != NULL);
    assert(xanList_find(list, item->data) != NULL);

    /* Initialize the variables.
     */
    prevItem = item->prev;
    nextItem = item->next;

    /* Update the last element of the list.
     */
    if (nextItem == NULL) {

        /* The item to delete is the last one of the list.
         * Therefore we have to update the lastItem field of list.
         */
        assert(list->lastItem == item);
        list->lastItem = prevItem;
    }

    /* Delete the item from the list*/
    if (prevItem == NULL) {

        /* The item is the first of the list	*/
        assert(list->firstItem == item);
        assert(prevItem == NULL);
        internal_freeItem(list, item);
        list->firstItem = nextItem;
        if (nextItem != NULL) {
            nextItem->prev = NULL;
        }
    } else {

        /* The item is not the first of the list*/
        assert(list->firstItem != item);
        assert(prevItem != NULL);
        assert(xanList_next(prevItem) == item);
        internal_freeItem(list, item);
        prevItem->next = nextItem;
        if (nextItem != NULL) {
            nextItem->prev = prevItem;
        }
    }
    (list->len)--;

    return;
}

void xanList_removeElementsContainedInTable (XanList *list, XanHashTable *elementsToRemove, JITBOOLEAN abortIfAnElementDoNotExist) {
    XanListItem *item;
    XanList     *toDelete;

    /* Assertions.
     */
    assert(list != NULL);

    /* Allocate the necessary memory.
     */
    toDelete    = xanList_new(list->alloc, list->free, list->clone);

    /* Identify the elements to remove contained in list.
     */
    item	    = xanList_first(list);
    while (item != NULL) {
        if (xanHashTable_lookup(elementsToRemove, item->data) != NULL){
            xanList_append(toDelete, item);
        }
        item	= item->next;
    }

    /* Remove elements.
     */
    item        = xanList_first(toDelete);
    while (item != NULL){
        xanList_deleteItem(list, item->data);
        item    = item->next;
    }
    if (abortIfAnElementDoNotExist){
        if (xanList_length(toDelete) != xanHashTable_elementsInside(elementsToRemove)){
            print_err("XANLIB: ERROR = There is no the element to delete from the list. ", 0);
            abort();
        }
    }

    /* Free the memory.
     */
    xanList_destroyList(toDelete);

    return ;
}

void xanList_removeElements (XanList *list, XanList *elementsToRemove, JITBOOLEAN abortIfAnElementDoNotExist) {
    XanListItem	*item;

    /* Assertions.
     */
    assert(list != NULL);

    item	= xanList_first(elementsToRemove);
    while (item != NULL) {
        xanList_removeElement(list, item->data, abortIfAnElementDoNotExist);
        item	= item->next;
    }

    return ;
}

void xanList_removeElement (XanList *list, void *data, JITBOOLEAN abortIfAnElementDoNotExist) {
    XanListItem     *item;
    XanListItem     *prevItem;
    XanListItem     *nextItem;

    /* Assertions			*/
    assert(list != NULL);

    /* Initialize the variables	*/
    item = NULL;
    prevItem = NULL;
    nextItem = NULL;

    /* Search the data.
     */
    item = xanList_find(list, data);
    if (item != NULL) {
        assert(item->data == data);
        prevItem = item->prev;
        nextItem = item->next;
    } else {
        if (abortIfAnElementDoNotExist) {
            print_err("XANLIB: ERROR = There is no the element to delete from the list. ", 0);
            abort();
        }
        return ;
    }
    assert(item != NULL);
    assert(item->data == data);

    /* Update the last element of the list  */
    if (nextItem == NULL) {

        /* The item to delete is the last one of the list.
         * Therefore we have to update the lastItem field of list       */
        assert(list->lastItem == item);
        list->lastItem = prevItem;
    }

    /* Delete the item from the list        */
    if (prevItem == NULL) {

        /* The item is the first of the list	*/
        assert(list->firstItem == item);
        assert(prevItem == NULL);
        internal_freeItem(list, item);
        list->firstItem = nextItem;
        if (nextItem != NULL) {
            nextItem->prev = NULL;
        }
    } else {

        /* The item is not the first of the list*/
        assert(list->firstItem != item);
        assert(prevItem != NULL);
        assert(xanList_next(prevItem) == item);
        internal_freeItem(list, item);
        prevItem->next = nextItem;
        if (nextItem != NULL) {
            nextItem->prev = prevItem;
        }
    }
    (list->len)--;

    return;
}

void xanList_removeAllElements (XanList *list, void *data) {
    XanListItem     *item;
    XanListItem     *prevItem;
    XanListItem     *nextItem;

    /* Assertions			*/
    assert(list != NULL);

    /* Initialize the variables	*/
    item = NULL;
    prevItem = NULL;
    nextItem = NULL;

    /* Search the data		*/
    item = xanList_find(list, data);
    while (item != NULL) {
        assert(item->data == data);
        prevItem = item->prev;
        nextItem = item->next;
        assert(item->data == data);

        /* Update the last element of the list  */
        if (nextItem == NULL) {

            /* The item to delete is the last one of the list.
             * Therefore we have to update the lastItem field of list       */
            assert(list->lastItem == item);
            list->lastItem = prevItem;
        }

        /* Delete the item from the list        */
        if (prevItem == NULL) {

            /* The item is the first of the list	*/
            assert(list->firstItem == item);
            assert(prevItem == NULL);
            internal_freeItem(list, item);
            list->firstItem = nextItem;
            if (nextItem != NULL) {
                nextItem->prev = NULL;
            }
        } else {

            /* The item is not the first of the list*/
            assert(list->firstItem != item);
            assert(prevItem != NULL);
            assert(xanList_next(prevItem) == item);
            internal_freeItem(list, item);
            prevItem->next = nextItem;
            if (nextItem != NULL) {
                nextItem->prev = prevItem;
            }
        }

        /* Decrease the length of the list	*/
        (list->len)--;

        /* Fetch the next element to delete	*/
        item = xanList_find(list, data);
    }

    return;
}

void xanList_deleteAndFreeElements (XanList *list) {
    XanListItem     *item;

    /* Assertions.
     */
    assert(list != NULL);

    item = xanList_first(list);
    while (item != NULL) {
        list->free(item->data);
        xanList_deleteItem(list, item);
        item = xanList_first(list);
    }
    assert(list->firstItem == NULL);
    assert(list->lastItem == NULL);

    return;
}

void xanList_deleteClones (XanList *list) {
    void            *data;
    void            *data2;
    XanListItem     *item;
    XanListItem     *item2;
    XanListItem     *item3;

    /* Assertions		*/
    assert(list != NULL);

    item = xanList_first(list);
    while (item != NULL) {
        data = item->data;
        item2 = item->next;
        while (item2 != NULL) {
            data2 = item2->data;
            if (data == data2) {
                item3 = item2->next;
                xanList_deleteItem(list, item2);
                item2 = item3;
            } else {
                item2 = item2->next;
            }
        }
        item = item->next;
    }

    return ;
}

void xanList_setCloneFunction (XanList *list, void * (*cloneFunction)(void *data)) {

    /* Assertions			*/
    assert(list != NULL);

    list->clone = cloneFunction;
}

void * xanList_getCloneFunction (XanList *list) {

    /* Assertions			*/
    assert(list != NULL);

    return list->clone;
}

XanList * xanList_cloneList (XanList *list) {
    XanList         *clone;
    XanListItem     *item;
    void            *cloneData;

    /* Assertions			*/
    assert(list != NULL);

    /* New list			*/
    clone = xanList_new(list->alloc, list->free, list->clone);
    assert(clone != NULL);

    /* Clone the list		*/
    item = xanList_first(list);
    while (item != NULL) {
        if (list->clone != NULL) {
            cloneData = list->clone(item->data);
        } else {
            cloneData = item->data;
        }
        xanList_append(clone, cloneData);
        item = item->next;
    }

    /* Return			*/
    return clone;
}

XanList * xanList_reverseList (XanList *list, bool clone) {
    XanList *reversed;
    XanListItem *item;
    XanListItem *temp;

    /* Assertions. */
    assert(list);

    /* Get the list to reverse. */
    if (clone) {
        reversed = xanList_cloneList(list);
    } else {
        reversed = list;
    }

    /* Reverse the list. */
    item = reversed->firstItem;
    while (item) {
        temp = item->next;
        item->next = item->prev;
        item->prev = temp;
        item = temp;
    }

    /* Update the list pointers. */
    temp = reversed->firstItem;
    reversed->firstItem = reversed->lastItem;
    reversed->lastItem = temp;

    /* Return the reversed list. */
    return reversed;
}

/**
 * Synchronised functions.
 **/

XanListItem * xanList_syncFirst (XanList *list) {
    XanListItem     *firstItem;

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    firstItem = xanList_first(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    /* Return the first item*/
    return firstItem;
}

XanListItem * xanList_syncLast (XanList *list) {
    XanListItem     *lastItem;

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    lastItem = xanList_last(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    /* Return the last item*/
    return lastItem;
}

XanListItem * xanList_syncNext (XanList *list, XanListItem *item) {
    XanListItem *nextItem;

    /* Assertions		*/
    assert(list != NULL);
    assert(item != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    nextItem = item->next;
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return nextItem;
}

XanListItem * xanList_syncPrev (XanList *list, XanListItem *item) {
    XanListItem *prevItem;

    /* Assertions		*/
    assert(list != NULL);
    assert(item != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    prevItem = item->prev;
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return prevItem;
}

XanListItem * xanList_syncAppend (XanList *list, void *data) {
    XanListItem     *item;

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    item = xanList_append(list, data);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return item;
}

void xanList_syncAppendList (XanList *list, XanList *listToAppend) {

    /* Assertions		*/
    assert(list != NULL);
    assert(listToAppend != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_appendList(list, listToAppend);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return;
}

XanListItem * xanList_syncInsert (XanList *list, void *data) {
    XanListItem     *item;

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    item = xanList_insert(list, data);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return item;
}

XanListItem * xanList_syncInsertBefore (XanList *list, XanListItem *prev, void *data) {
    XanListItem     *item;

    /* Assertions		*/
    assert(list != NULL);
    assert(prev != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    item = xanList_insertBefore(list, prev, data);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return item;
}

XanListItem * xanList_syncInsertAfter (XanList *list, XanListItem *prev, void *data) {
    XanListItem     *item;

    /* Assertions		*/
    assert(list != NULL);
    assert(prev != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    item = xanList_syncInsertAfter(list, prev, data);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return item;
}

XanListItem * xanList_syncMoveToBegin (XanList *list, XanListItem *item) {
    XanListItem     *itemToMove;

    /* Assertions			*/
    assert(list != NULL);
    assert(item != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    itemToMove = xanList_moveToBegin(list, item);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return itemToMove;
}

XanListItem * xanList_syncFind (XanList *list, void *data) {
    XanListItem     *item;

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    item = xanList_find(list, data);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    /* Return the first item*/
    return item;
}

void * xanList_syncGetData (XanList *list, XanListItem *item) {
    void *data;

    /* Assertions		*/
    assert(list != NULL);
    assert(item != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    data = item->data;
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return data;
}

int xanList_syncLength (XanList *list) {
    int length;

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    length = xanList_length(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    /* Return			*/
    return length;
}

XanListItem * xanList_syncGetElementFromPositionNumber (XanList *list, int positionNumber) {
    XanListItem     *item;

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    item = xanList_getElementFromPositionNumber(list, positionNumber);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return item;
}

bool xanList_syncShareSomeElements (XanList *list, XanList *otherList) {
    bool result;

    /* Assertions		*/
    assert(list != NULL);
    assert(otherList != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    result = xanList_shareSomeElements(list, otherList);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return result;
}

bool xanList_syncContainsTheSameElements (XanList *list, XanList *otherList) {
    bool result;

    /* Assertions		*/
    assert(list != NULL);

    /* This could deadlock! */

    PLATFORM_lockMutex(&(list->itemsMutex));
    result = false;
    if (otherList != NULL) {
        PLATFORM_lockMutex(&(otherList->itemsMutex));
        result = xanList_containsTheSameElements(list, otherList);
        PLATFORM_unlockMutex(&(otherList->itemsMutex));
    }
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return result;
}

int xanList_syncEqualsInstancesNumber (XanList *list, void *data) {
    int num;

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    num = xanList_equalsInstancesNumber(list, data);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return num;
}

void ** xanList_syncGetSlotData (XanList *list, XanListItem *item) {
    void **data;

    /* Assertions		*/
    assert(list != NULL);
    assert(item != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    data = xanList_getSlotData(item);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return data;
}

void xanList_syncEmptyOutList (XanList *list) {
    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_emptyOutList(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));
}

/**
 * Does it make sense to have synchonised versions of list destroying functions?
 **/

void xanList_syncDestroyList (XanList *list) {

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_destroyList(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));
}

void xanList_syncDestroyListAndData (XanList *list) {

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_destroyListAndData(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));
}

void xanList_syncDeleteItem (XanList *list, XanListItem *item) {

    /* Assertions		*/
    assert(list != NULL);
    assert(item != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_deleteItem(list, item);
    PLATFORM_unlockMutex(&(list->itemsMutex));
}

void xanList_syncRemoveElement (XanList *list, void *data) {

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_removeElement(list, data, JITTRUE);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return ;
}

void xanList_syncRemoveAllElements (XanList *list, void *data) {

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_removeAllElements(list, data);
    PLATFORM_unlockMutex(&(list->itemsMutex));
}

void xanList_syncDeleteClones (XanList *list) {

    /* Assertions		*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_deleteClones(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return;
}

void xanList_syncSetCloneFunction (XanList *list, void * (*cloneFunction)(void *data)) {

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    xanList_setCloneFunction(list, cloneFunction);
    PLATFORM_unlockMutex(&(list->itemsMutex));
}

void * xanList_syncGetCloneFunction (XanList *list) {
    void    *func;

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
    func = xanList_getCloneFunction(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    return func;
}

XanList * xanList_syncCloneList (XanList *list) {
    XanList *clone;

    /* Assertions			*/
    assert(list != NULL);

    /* Clone the list		*/
    PLATFORM_lockMutex(&(list->itemsMutex));
    clone = xanList_cloneList(list);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    /* Return                       */
    return clone;
}

XanList * xanList_syncReverseList (XanList *list, bool clone) {
    XanList *reversed;

    /* Assertions			*/
    assert(list != NULL);

    /* Clone the list		*/
    PLATFORM_lockMutex(&(list->itemsMutex));
    reversed = xanList_reverseList(list, clone);
    PLATFORM_unlockMutex(&(list->itemsMutex));

    /* Return                       */
    return reversed;
}

void xanList_lock (XanList *list) {

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_lockMutex(&(list->itemsMutex));
}

void xanList_unlock (XanList *list) {

    /* Assertions			*/
    assert(list != NULL);

    PLATFORM_unlockMutex(&(list->itemsMutex));
}

static inline XanListItem * internal_allocateItem (XanList *l){
    XanListItem *i;

    /* Check if we have a buffer and it is not empty.
     */
    if (    (l->bufferItems == NULL)    ||
            (l->bufferItemsTop == 0)    ){

        /* We do not have a buffer or the buffer is empty.
         * Hence, we have to allocate a new item.
         */
        return l->alloc(sizeof(XanListItem));
    }

    /* We do have a buffer and it has some item to take.
     * Take an item.
     */
    i   = l->bufferItems[l->bufferItemsTop - 1];
    (l->bufferItemsTop)--;
    assert(i != NULL);

    return i;
}

static inline void internal_freeItem (XanList *l, XanListItem *item){

    /* Check if we have to allocate the buffer of items.
     */
    if (l->bufferItems == NULL){
        l->bufferItemsSize  = 4;
        l->bufferItems      = l->alloc(sizeof(XanListItem *) * (l->bufferItemsSize));
    }

    /* Check whether there is any space left.
     */
    if (l->bufferItemsTop == l->bufferItemsSize){

        /* There is no space left; hence, we cannot buffer the current item.
         * Free the current item and return.
         */
        l->free(item);
        return ;
    }

    /* Erase the item.
     */
    memset(item, 0, sizeof(XanListItem));

    /* Move the current item to the buffer.
     */
    l->bufferItems[l->bufferItemsTop]   = item;
    (l->bufferItemsTop)++;

    return ;
}

static inline void internal_destroyBuffer (XanList *list){

    if (list->bufferItems != NULL){
        JITUINT32 count;
        for (count=0; count < list->bufferItemsTop; count++){
            list->free(list->bufferItems[count]);
        }
        list->free(list->bufferItems);
    }

    return ;
}
