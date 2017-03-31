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
 * Helper functions.
 **/

#define DEFAULT_HEAP_SIZE 15
#define HEAP_IS_ROOT(r) (r==0)
#define HEAP_IS_LEAF(queue, r) (r>=((queue->size)/2))
#define HEAP_PARENT(r) (unsigned int) ((r-1)/2)
#define HEAP_LEFT_CHILD(r) (unsigned int) ((2*r)+1)
#define HEAP_RIGHT_CHILD(r) (unsigned int) ((2*r)+2)
#define HEAP_SWAP_ITEM(items, p1, p2) \
	{ \
		XanQueueItem *from = items[p1];	\
		XanQueueItem *to = items[p2]; \
		items[p2] = from; \
		items[p2]->position = p2; \
		items[p1] = to;	\
		items[p1]->position = p1; \
	}

static inline int heapShiftDown (XanQueue *queue, int position) {
    while (!HEAP_IS_LEAF(queue, position)) {
        int left_child = HEAP_LEFT_CHILD(position);
        int right_child = HEAP_RIGHT_CHILD(position);
        int largest = position;
        if ((queue->items[left_child]) == NULL) {
            largest = right_child;
        } else if ((queue->items[right_child]) == NULL) {
            largest = left_child;
        } else if ((queue->items[left_child])->priority >= (queue->items[right_child])->priority) {
            largest = left_child;
        } else if ((queue->items[left_child])->priority <= (queue->items[right_child])->priority) {
            largest = right_child;
        }
        if ((queue->items[largest])->priority <= (queue->items[position])->priority) {
            break;
        }
        HEAP_SWAP_ITEM(queue->items, position, largest);
        position = largest;
    }
    return position;
}

static inline int heapShiftUp (XanQueue *queue, int position) {
    while (!HEAP_IS_ROOT(position)) {
        int newPosition = HEAP_PARENT(position);
        if ((queue->items[position])->priority <= (queue->items[newPosition])->priority) {
            break;
        }
        HEAP_SWAP_ITEM(queue->items, position, newPosition);
        position = newPosition;
    }
    return position;
}

static inline void *heapExtractItem (XanQueue *queue, int position) {
    void *result;

    assert(queue->size > position);
    assert(queue->items[position]->position == position);
    if (position!=queue->size-1) {
        (queue->size)--;
        HEAP_SWAP_ITEM(queue->items, position, queue->size);
        result = (queue->items[queue->size])->element;
        queue->free(queue->items[queue->size]);
        queue->items[queue->size] = NULL;
        assert(queue->size > position);
        assert(queue->items[position]->position == position);
        position = heapShiftUp(queue, position);
        assert(queue->size > position);
        assert(queue->items[position]->position == position);
        position = heapShiftDown(queue, position);
        assert(queue->size > position);
        assert(queue->items[position]->position == position);
    } else {
        (queue->size)--;
        result = (queue->items[queue->size])->element;
        queue->free(queue->items[queue->size]);
        queue->items[queue->size] = NULL;
    }
    return result;
}


/**
 * Unsynchronised functions.
 **/

XanQueue * xanQueue_new (void *(*allocFunction)(size_t size), void *(*reallocFunction)(void *addr, size_t newSize), void (*freeFunction)(void *addr)) {
    XanQueue        *newQueue;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        newQueue		= malloc(sizeof(XanQueue));
        newQueue->alloc	= malloc;
    } else {
        newQueue		= allocFunction(sizeof(XanQueue));
        newQueue->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        newQueue->free		= free;
    } else {
        newQueue->free		= freeFunction;
    }
    if (reallocFunction == NULL) {
        newQueue->realloc	= realloc;
    } else {
        newQueue->realloc	= reallocFunction;
    }

    /* Initialize the queue			*/
    assert(newQueue != NULL);
    newQueue->items = allocFunction(sizeof(XanQueueItem *) * DEFAULT_HEAP_SIZE);
    assert(newQueue->items != NULL);
    memset(newQueue->items, 0, sizeof(XanQueueItem *) * DEFAULT_HEAP_SIZE);
    newQueue->maxSize = DEFAULT_HEAP_SIZE;
    newQueue->size = 0;
    assert(newQueue->items != NULL);
    pthread_mutexattr_t mutex_attr;
    pthread_condattr_t cond_attr;
    PLATFORM_initMutexAttr(&mutex_attr);
    PLATFORM_initCondVarAttr(&cond_attr);
    PLATFORM_setMutexAttr_type(&mutex_attr, PTHREAD_MUTEX_ADAPTIVE_NP);
#ifdef MULTIAPP
    PLATFORM_setMutexAttr_pshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    PLATFORM_setCondVarAttr_pshared(&cond_attr, PTHREAD_PROCESS_SHARED);
#endif
    PLATFORM_initMutex(&(newQueue->itemsMutex), &mutex_attr);
    PLATFORM_initCondVar(&(newQueue->itemsCond), &cond_attr);

    /* Return the queue			*/
    return newQueue;
}

void * xanQueue_get (XanQueue *queue) {
    void            *data;

    /* Assertions		*/
    assert(queue != NULL);
    assert(queue->items != NULL);

    /* Fetch the first item	*
     * in the list		*/
    PLATFORM_lockMutex(&(queue->itemsMutex));
    while (queue->size == 0) {
        PLATFORM_waitCondVar(&(queue->itemsCond), &(queue->itemsMutex));
    }
    PLATFORM_unlockMutex(&(queue->itemsMutex));
    data = heapExtractItem(queue,0);

    /* Return the data	*/
    return data;
}

XanQueueItem * xanQueue_put (XanQueue *queue, void *data, float priority) {
    XanQueueItem *result;

    /* Assertions				*/
    assert(queue != NULL);
    assert(queue->items != NULL);

    if (queue->size == queue->maxSize) {
        queue->items = queue->realloc(queue->items, (2*(queue->size+1)-1) *  sizeof(XanQueue *));
        assert(queue->items != NULL);
        memset(&(queue->items[queue->maxSize]), 0, sizeof(XanQueue *) * (queue->size+1));
        queue->maxSize += (queue->size+1);
    }

    int position = queue->size++;
    result = (XanQueueItem *) queue->alloc(sizeof(XanQueueItem));
    result->priority = priority;
    result->position = position;
    result->element = data;
    queue->items[position] = result;
    position = heapShiftUp(queue, position);
    position = heapShiftDown(queue, position);
    assert(queue->size > position);
    assert(queue->items[position]->position == position);

    /* Send the signal			*/
    if (queue->size == 1) {
        PLATFORM_broadcastCondVar(&(queue->itemsCond));
    }

    /* Return				*/
    return result;
}

XanQueueItem * xanQueue_find (XanQueue *queue, void *data) {
    XanQueueItem *result = NULL;
    int count;

    for (count = 0; count < queue->maxSize; count++) {
        if ((queue->items[count])!=NULL && (queue->items[count])->element==data) {
            result = queue->items[count];
            break;
        }
    }

    return result;
}

bool xanQueue_isEmpty (XanQueue *queue) {

    /* Assertions				*/
    assert(queue != NULL);

    return queue->size == 0;
}

float xanQueue_headPriority (XanQueue *queue) {

    /* Assertions				*/
    assert(queue != NULL);

    if (queue->size == 0) {
        return -1.0;
    }
    return queue->items[0]->priority;
}

void xanQueue_changeItemPriority (XanQueue *queue, XanQueueItem *handle, float priority) {
    int position = handle->position;

    handle->priority = priority;

    assert(queue->size > position);
    assert(queue->items[position]->position == position);
    position = heapShiftUp(queue, position);
    assert(queue->size > position);
    assert(queue->items[position]->position == position);
    position = heapShiftDown(queue, position);
    assert(queue->size > position);
    assert(queue->items[position]->position == position);
}

void xanQueue_removeItem (XanQueue *queue, XanQueueItem *handle) {

    /* Assertions				*/
    assert(queue != NULL);
    assert(handle != NULL);

    assert(queue->size > handle->position);
    heapExtractItem(queue, handle->position);
}

void xanQueue_destroyQueue (XanQueue *queue) {

    /* Assertions				*/
    assert(queue != NULL);
    assert(queue->items != NULL);

    queue->free(queue->items);
    queue->free(queue);
}

void xanQueue_lock (XanQueue *queue) {

    /* Assertions				*/
    assert(queue != NULL);

    PLATFORM_lockMutex(&(queue->itemsMutex));
}

void xanQueue_unlock (XanQueue *queue) {

    /* Assertions				*/
    assert(queue != NULL);

    PLATFORM_unlockMutex(&(queue->itemsMutex));
}


/**
 * Synchronised functions.
 **/

void * xanQueue_syncGet (XanQueue *queue) {
    void            *data;

    /* Assertions		*/
    assert(queue != NULL);
    assert(queue->items != NULL);

    /* Fetch the first item	*
     * in the list		*/
    PLATFORM_lockMutex(&(queue->itemsMutex));
    while (queue->size == 0) {
        PLATFORM_waitCondVar(&(queue->itemsCond), &(queue->itemsMutex));
    }
    assert(queue->size>0);
    data = heapExtractItem(queue,0);
    PLATFORM_unlockMutex(&(queue->itemsMutex));

    /* Return the data	*/
    return data;
}

XanQueueItem * xanQueue_syncPut (XanQueue *queue, void *item, float priority) {
    XanQueueItem    *queueItem;

    PLATFORM_lockMutex(&(queue->itemsMutex));
    queueItem = xanQueue_put(queue, item, priority);
    PLATFORM_unlockMutex(&(queue->itemsMutex));

    return queueItem;
}

XanQueueItem *xanQueue_syncFind (XanQueue *queue, void *element) {
    XanQueueItem    *queueItem;

    PLATFORM_lockMutex(&(queue->itemsMutex));
    queueItem = xanQueue_find(queue, element);
    PLATFORM_unlockMutex(&(queue->itemsMutex));

    return queueItem;
}

bool xanQueue_syncIsEmpty (XanQueue *queue) {
    bool result;

    /* Assertions				*/
    assert(queue != NULL);

    PLATFORM_lockMutex(&(queue->itemsMutex));
    result = xanQueue_isEmpty(queue);
    PLATFORM_unlockMutex(&(queue->itemsMutex));
    return result;
}

float xanQueue_syncHeadPriority (XanQueue *queue) {
    float result;

    /* Assertions				*/
    assert(queue != NULL);

    PLATFORM_lockMutex(&(queue->itemsMutex));
    result = xanQueue_headPriority(queue);
    PLATFORM_unlockMutex(&(queue->itemsMutex));
    return result;
}

void xanQueue_syncChangeItemPriority (XanQueue *queue, XanQueueItem *item, float priority) {
    /* Assertions				*/
    assert(queue != NULL);

    PLATFORM_lockMutex(&(queue->itemsMutex));
    xanQueue_changeItemPriority(queue, item, priority);
    PLATFORM_unlockMutex(&(queue->itemsMutex));
}

void xanQueue_syncRemoveItem (XanQueue *queue, XanQueueItem *element) {

    PLATFORM_lockMutex(&(queue->itemsMutex));
    xanQueue_removeItem(queue, element);
    PLATFORM_unlockMutex(&(queue->itemsMutex));
}
