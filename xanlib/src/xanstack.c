/*
 * Copyright (C) 2007 - 2010 Campanoni Simone, Luca Rocchini, Michele Tartara
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


/**
 * Unsynchronised functions.
 **/

XanStack * xanStack_new (void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void * (*cloneFunction)(void *data)) {
    XanStack        	*stack;
    pthread_mutexattr_t 	mutex_attr;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        stack		= malloc(sizeof(XanStack));
        stack->alloc	= malloc;
    } else {
        stack		= allocFunction(sizeof(XanStack));
        stack->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        stack->free		= free;
    } else {
        stack->free		= freeFunction;
    }

    /* Initialize the stack			*/
    assert(stack != NULL);
    stack->internalList = xanList_new(allocFunction, freeFunction, cloneFunction);
    assert(stack->internalList != NULL);
    PLATFORM_initMutexAttr(&mutex_attr);
    PLATFORM_setMutexAttr_type(&mutex_attr, PTHREAD_MUTEX_ADAPTIVE_NP);
#ifdef MULTIAPP
    PLATFORM_setMutexAttr_pshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
#endif
    PLATFORM_initMutex(&(stack->mutex), &mutex_attr);
    stack->clone	= cloneFunction;

    /* Return				*/
    return stack;
}

void xanStack_push (XanStack *stack, void *newElement) {

    /* Assertions		*/
    assert(stack != NULL);

    xanList_insert(stack->internalList, newElement);
}

void * xanStack_pop (XanStack *stack) {
    XanListItem     *item;
    void            *element;

    /* Assertions		*/
    assert(stack != NULL);

    element = NULL;

    /* Fetch the element	*/
    if (xanList_length(stack->internalList) > 0) {
        item = xanList_first(stack->internalList);
        assert(item != NULL);
        element = item->data;
        xanList_deleteItem(stack->internalList, item);
    }

    /* Return		*/
    return element;
}

void * xanStack_top (XanStack *stack) {
    XanListItem     *item;
    void            *element;

    /* Assertions			*/
    assert(stack != NULL);

    /* Check if there are elements	*
     * within the list		*/
    if (xanList_length(stack->internalList) == 0) {
        print_err("XanStack: ERROR = The stack is empty while reading its top. ", 0);
        abort();
    }

    /* Fetch the element		*/
    item = xanList_first(stack->internalList);
    assert(item != NULL);
    element = item->data;

    /* Return			*/
    return element;
}

int xanStack_getSize (XanStack *stack) {
    int size;

    /* Assertions		*/
    assert(stack != NULL);

    size = xanList_length(stack->internalList);

    return size;
}

bool xanStack_contains (XanStack *stack, void *element) {
    void* elemFound;

    /* Assertions		*/
    assert(stack != NULL);

    elemFound = xanList_find(stack->internalList, element);

    if (elemFound==NULL) {
        return false;
    } else {
        return true;
    }
}

bool xanStack_containsTheSameElements (XanStack *stack1, XanStack *stack2) {

    if (stack1 == stack2) {
        return true;
    }
    if (	(stack1 == NULL)	||
            (stack2 == NULL)	) {
        return false;
    }
    return xanList_containsTheSameElements(stack1->internalList, stack2->internalList);
}

XanList *
xanStack_toList(XanStack *stack) {
    return xanList_cloneList(stack->internalList);
}

XanStack * xanStack_clone (XanStack *stack) {
    XanStack *clone;

    /* Check the stack	*/
    if (stack == NULL) {
        return NULL;
    }

    /* Clone the stack	*/
    clone = xanStack_new(stack->alloc, stack->free, stack->clone);
    assert(clone != NULL);
    xanList_destroyList(clone->internalList);
    clone->internalList	= xanList_cloneList(stack->internalList);

    /* Return		*/
    return clone;
}

void xanStack_destroyStack (XanStack *stack) {

    /* Assertions		*/
    assert(stack != NULL);
    assert(stack->internalList != NULL);

    xanList_destroyList(stack->internalList);
    PLATFORM_destroyMutex(&(stack->mutex));
    free(stack);
}


/**
 * Synchronised functions.
 **/

void xanStack_syncPush (XanStack *stack, void *newElement) {

    /* Assertions		*/
    assert(stack != NULL);

    PLATFORM_lockMutex(&(stack->mutex));
    xanStack_push(stack, newElement);
    PLATFORM_unlockMutex(&(stack->mutex));

    return;
}

void * xanStack_syncPop (XanStack *stack) {
    void            *element;

    /* Assertions		*/
    assert(stack != NULL);

    PLATFORM_lockMutex(&(stack->mutex));
    element = xanStack_pop(stack);
    PLATFORM_unlockMutex(&(stack->mutex));

    return element;
}

void * xanStack_syncTop (XanStack *stack) {
    void            *element;

    /* Assertions		*/
    assert(stack != NULL);

    PLATFORM_lockMutex(&(stack->mutex));
    element = xanStack_top(stack);
    PLATFORM_unlockMutex(&(stack->mutex));

    return element;
}

int xanStack_syncGetSize (XanStack *stack) {
    int size;

    /* Assertions		*/
    assert(stack != NULL);

    PLATFORM_lockMutex(&(stack->mutex));
    size = xanStack_getSize(stack);
    PLATFORM_unlockMutex(&(stack->mutex));

    return size;
}

bool xanStack_synchContains (XanStack *stack, void *element) {
    bool res;

    /* Assertions		*/
    assert(stack != NULL);

    PLATFORM_lockMutex(&(stack->mutex));
    res = xanStack_contains(stack, element);
    PLATFORM_unlockMutex(&(stack->mutex));

    return res;
}
