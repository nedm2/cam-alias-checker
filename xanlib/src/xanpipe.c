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

XanPipe * xanPipe_new (void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr)) {
    XanPipe *newPipe;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        newPipe		= malloc(sizeof(XanPipe));
        newPipe->alloc	= malloc;
    } else {
        newPipe		= allocFunction(sizeof(XanPipe));
        newPipe->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        newPipe->free		= free;
    } else {
        newPipe->free		= freeFunction;
    }
    /* Initialize the pipe			*/
    assert(newPipe != NULL);
    newPipe->items = xanList_new(newPipe->alloc, newPipe->free, NULL);
    assert(newPipe->items != NULL);
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    PLATFORM_initMutexAttr(&mutex_attr);
    PLATFORM_initCondVarAttr(&cond_attr);
    PLATFORM_setMutexAttr_type(&mutex_attr, PTHREAD_MUTEX_ADAPTIVE_NP);
#ifdef MULTIAPP
    PLATFORM_setMutexAttr_pshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    PLATFORM_setCondVarAttr_pshared(&cond_attr, PTHREAD_PROCESS_SHARED);
#endif
    PLATFORM_initMutex(&(newPipe->itemsMutex), &mutex_attr);
    PLATFORM_initCondVar(&(newPipe->itemsCond), &cond_attr);

    /* Return the pipe			*/
    return newPipe;
}

void * xanPipe_get (XanPipe *pipe) {
    XanListItem     *item;
    void            *data;

    /* Assertions		*/
    assert(pipe != NULL);
    assert(pipe->items != NULL);

    /* Initialize the variables	*/
    data = NULL;
    item = NULL;

    /* Fetch the first item	*
     * in the list		*/
    PLATFORM_lockMutex(&(pipe->itemsMutex));
    while (xanList_length(pipe->items) == 0) {
        PLATFORM_waitCondVar(&(pipe->itemsCond), &(pipe->itemsMutex));
    }
    PLATFORM_unlockMutex(&(pipe->itemsMutex));
    item = xanList_first(pipe->items);
    assert(item != NULL);
    data = item->data;
    xanList_deleteItem(pipe->items, item);

    /* Return the data	*/
    return data;
}

XanListItem * xanPipe_put (XanPipe *pipe, void *item) {
    XanListItem     *pipeItem;

    /* Assertions				*/
    assert(pipe != NULL);
    assert(pipe->items != NULL);
    PDEBUG("XANLIB: xanPipePut: Start\n");

    /* Insert the item at the end of the    *
    * list                                 */
    PDEBUG("XANLIB: xanPipePut:     Inserted the new element\n");
    pipeItem = xanList_append(pipe->items, item);
    assert(pipeItem != NULL);
    assert(xanList_getData(pipeItem) == item);

    /* Send the signal			*/
    if (xanList_length(pipe->items) == 1) {
        PLATFORM_broadcastCondVar(&(pipe->itemsCond));
    }

    /* Return				*/
    PDEBUG("XANLIB: xanPipePut: Exit\n");
    return pipeItem;
}

XanListItem * xanPipe_putAtEnd (XanPipe *pipe, void *item) {
    XanListItem     *pipeItem;

    /* Assertions				*/
    assert(pipe != NULL);
    assert(pipe->items != NULL);
    assert(item != NULL);
    PDEBUG("XANLIB: xanPipePutAtEnd: Start\n");

    /* Insert the item at the end of the    *
    * list                                 */
    PDEBUG("XANLIB: xanPipePutAtEnd:        Inserted the new element\n");
    pipeItem = xanList_insert(pipe->items, item);
    assert(pipeItem != NULL);
    assert(xanList_getData(pipeItem) == item);
    PLATFORM_signalCondVar(&(pipe->itemsCond));

    /* Return				*/
    PDEBUG("XANLIB: xanPipePutAtEnd: Exit\n");
    return pipeItem;
}

XanListItem * xanPipe_moveToEnd (XanPipe *pipe, void *data) {
    XanListItem     *itemList;

    /* Assertions				*/
    assert(pipe != NULL);
    assert(pipe->items != NULL);
    assert(data != NULL);
    PDEBUG("XANLIB: xanPipeMoveToEnd: Start\n");

    /* Insert the item at the end of the    *
    * list                                 */
    PDEBUG("XANLIB: xanPipeMoveToEnd:       Move to the end of the pipe the element\n");
    itemList = xanList_find(pipe->items, data);
    if (itemList != NULL) {
        assert(xanList_equalsInstancesNumber(pipe->items, data) == 1);
        assert(xanList_length(pipe->items) > 0);
        assert(xanList_getData(itemList) == data);
        itemList = xanList_moveToBegin(pipe->items, itemList);
        assert(xanList_getData(itemList) == data);
        assert(xanList_length(pipe->items) > 0);
        assert(xanList_equalsInstancesNumber(pipe->items, data) == 1);
    }

    /* Return				*/
    PDEBUG("XANLIB: xanPipeMoveToEnd: Exit\n");
    return itemList;
}

void xanPipe_removeItem (XanPipe *pipe, XanListItem *handle) {

    /* Assertions				*/
    assert(pipe != NULL);
    assert(handle != NULL);

    xanList_deleteItem(pipe->items, handle);
}

bool xanPipe_isEmpty (XanPipe *pipe) {

    /* Assertions				*/
    assert(pipe != NULL);

    return xanList_length(pipe->items) == 0;
}

void xanPipe_destroyPipe (XanPipe *pipe) {

    /* Assertions				*/
    assert(pipe != NULL);
    assert(pipe->items != NULL);

    xanList_destroyList(pipe->items);
}

void xanPipe_lock (XanPipe *pipe) {

    /* Assertions				*/
    assert(pipe != NULL);

    PLATFORM_lockMutex(&(pipe->itemsMutex));
}

void xanPipe_unlock (XanPipe *pipe) {

    /* Assertions				*/
    assert(pipe != NULL);

    PLATFORM_unlockMutex(&(pipe->itemsMutex));
}


/**
 * Synchronised functions.
 **/

void * xanPipe_syncGet (XanPipe *pipe) {
    XanListItem     *item;
    void            *data;

    /* Assertions		*/
    assert(pipe != NULL);
    assert(pipe->items != NULL);

    /* Initialize the variables	*/
    data = NULL;
    item = NULL;

    /* Fetch the first item	*
     * in the list		*/
    PLATFORM_lockMutex(&(pipe->itemsMutex));
    while (xanList_length(pipe->items) == 0) {
        PLATFORM_waitCondVar(&(pipe->itemsCond), &(pipe->itemsMutex));
    }
    item = xanList_first(pipe->items);
    assert(item != NULL);
    data = item->data;
    xanList_deleteItem(pipe->items, item);

    /* Unlock		*/
    PLATFORM_unlockMutex(&(pipe->itemsMutex));

    /* Return the data	*/
    return data;
}

XanListItem * xanPipe_syncPut (XanPipe *pipe, void *item) {
    XanListItem     *pipeItem;

    PLATFORM_lockMutex(&(pipe->itemsMutex));
    pipeItem = xanPipe_put(pipe, item);
    PLATFORM_unlockMutex(&(pipe->itemsMutex));

    return pipeItem;
}

XanListItem * xanPipe_syncPutAtEnd (XanPipe *pipe, void *item) {
    XanListItem     *pipeItem;

    PLATFORM_lockMutex(&(pipe->itemsMutex));
    pipeItem = xanPipe_putAtEnd(pipe, item);
    PLATFORM_unlockMutex(&(pipe->itemsMutex));

    return pipeItem;
}

XanListItem * xanPipe_syncMoveToEnd (XanPipe *pipe, void *data) {
    XanListItem     *pipeItem;

    PLATFORM_lockMutex(&(pipe->itemsMutex));
    pipeItem = xanPipe_moveToEnd(pipe, data);
    PLATFORM_unlockMutex(&(pipe->itemsMutex));

    return pipeItem;
}

void xanPipe_syncRemoveItem (XanPipe *pipe, XanListItem *handle) {

    PLATFORM_lockMutex(&(pipe->itemsMutex));
    xanPipe_removeItem(pipe, handle);
    PLATFORM_unlockMutex(&(pipe->itemsMutex));
}

bool xanPipe_syncIsEmpty (XanPipe *pipe) {
    int result;

    /* Assertions				*/
    assert(pipe != NULL);

    PLATFORM_lockMutex(&(pipe->itemsMutex));
    result = xanPipe_isEmpty(pipe);
    PLATFORM_unlockMutex(&(pipe->itemsMutex));
    return result;
}
