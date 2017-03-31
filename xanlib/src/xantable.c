/*
 * Copyright (C) 2007 - 2012 Campanoni Simone
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


/* XanHashTable expand parameters */
static const size_t primes[] = {
    53,	   97,	       193,	    389,
    769,	   1543,       3079,	    6151,
    12289,	   24593,      49157,	    98317,
    196613,	   393241,     786433,	    1572869,
    3145739,   6291469,    12582917,    25165843,
    50331653,  100663319,  201326611,   402653189,
    805306457, 1610612741
};
#define PRIMES_TABLE_LENGTH (sizeof(primes)/sizeof(primes[0]))
#define HASHTABLE_DEFAULT_LOAD_FACTOR 0.75

static inline int xanHashTableDefaultEqualsFunction (void *key1, void *key2) {
    if (key1 == key2) {
        return 1;
    }
    return 0;
}

static inline unsigned int xanHashTableDefaultHashFunction (void *element) {
    return (unsigned int )(JITNUINT) element;
}

static inline unsigned int _xanHashTableImproveHash (XanHashTable* table, void *key) {
    void *keyToUse;
    unsigned int i;

    /* Fetch the key to use		*/
    keyToUse = key;

    /* Compute the hash		*/
    i = table->hash(keyToUse);

    /* Improve the hash		*/
    i += ~(i << 9);
    i ^= ((i >> 14) | (i << 18));
    i += (i << 4);
    i ^= ((i >> 10) | (i << 22));

    /* Return			*/
    return i;
}

static inline void _xanHashTableExpand (XanHashTable* table) {
    XanHashTableItem* newTable;
    XanHashTableItem* bucket;
    XanHashTableItem* newBucket;
    XanHashTableItem* next;
    unsigned int newPrimeIndex;
    size_t newLength;
    size_t i;
    size_t newIndex;
    size_t elementsCount;

    /* Alloc a new empty table */
    newPrimeIndex 	= table->primeIndex+1;
    newLength 	= primes[newPrimeIndex];
    newTable 	= (XanHashTableItem*) table->alloc(sizeof(XanHashTableItem) * newLength);
    memset(newTable, 0, sizeof(XanHashTableItem) * newLength);
    elementsCount 	= 0;

    /* Rehash the table */
    for (i = 0; i < table->length; i++) {
        bucket = &(table->table[i]);
        if (!bucket->used) {
            continue;
        }

        /* Insert the element to the new table	*/
        newIndex = _xanHashTableImproveHash(table, bucket->elementID) % newLength;
        newBucket = &(newTable[newIndex]);
        if (!newBucket->used) {
            newBucket->index = newIndex;
            newBucket->elementID = bucket->elementID;
            newBucket->element = bucket->element;
            newBucket->used = true;
            newBucket->next = NULL;
            elementsCount++;
        } else {
            XanHashTableItem *item = table->alloc(sizeof(XanHashTableItem));
            item->index = newIndex;
            item->elementID = bucket->elementID;
            item->element = bucket->element;
            item->used = true;
            item->next = NULL;
            while (newBucket->next!=NULL) {
                newBucket = newBucket->next;
            }
            newBucket->next = item;
        }

        /* Insert every element overflowed in the old table to the new one */
        bucket = bucket->next;
        while (bucket!=NULL) {
            newIndex = _xanHashTableImproveHash(table, bucket->elementID) % newLength;
            newBucket = &(newTable[newIndex]);
            if (!newBucket->used) {
                newBucket->index = newIndex;
                newBucket->elementID = bucket->elementID;
                newBucket->element = bucket->element;
                newBucket->used = true;
                newBucket->next = NULL;
                elementsCount++;
            } else {
                XanHashTableItem *item = table->alloc(sizeof(XanHashTableItem));
                item->index = newIndex;
                item->elementID = bucket->elementID;
                item->element = bucket->element;
                item->used = true;
                item->next = NULL;
                while (newBucket->next!=NULL) {
                    newBucket = newBucket->next;
                }
                newBucket->next = item;
            }
            next = bucket->next;
            table->free(bucket);
            bucket = next;
        }
    }

    /* Update table statistics */
    table->free(table->table);
    table->table = newTable;
    table->length = newLength;
    table->primeIndex = newPrimeIndex;
    table->currentLoadFactor = ((float) elementsCount / newLength);

    /* Return			*/
    return;
}

XanHashTable * xanHashTable_new (unsigned int length, int hasFixedLength, void *(*allocFunction)(size_t size), void *(*reallocFunction)(void *addr, size_t newSize), void (*freeFunction)(void *addr), unsigned int (*hashFunction)(void *element), int (*equalsFunction)(void *key1, void *key2)) {
    XanHashTable    *hashTable;
    unsigned int count;

    /* Allocate the structure		*/
    if (allocFunction == NULL) {
        hashTable		= malloc(sizeof(XanHashTable));
        hashTable->alloc	= malloc;
    } else {
        hashTable		= allocFunction(sizeof(XanHashTable));
        hashTable->alloc	= allocFunction;
    }
    if (freeFunction == NULL) {
        hashTable->free		= free;
    } else {
        hashTable->free		= freeFunction;
    }
    if (reallocFunction == NULL) {
        hashTable->realloc	= realloc;
    } else {
        hashTable->realloc	= reallocFunction;
    }

    /* Initialize the table	*/
    hashTable->clone = NULL;
    if (hashFunction == NULL) {
        hashTable->hash = xanHashTableDefaultHashFunction;
    } else {
        hashTable->hash = hashFunction;
    }
    if (equalsFunction == NULL) {
        hashTable->equals = xanHashTableDefaultEqualsFunction;
    } else {
        hashTable->equals = equalsFunction;
    }
    for (count = 0; count < PRIMES_TABLE_LENGTH; count++) {
        if (primes[count] > length) {
            hashTable->primeIndex = count;
            hashTable->length = primes[count];
            break;
        }
    }
    hashTable->table = allocFunction(sizeof(XanHashTableItem) * hashTable->length);
    memset(hashTable->table, 0, sizeof(XanHashTableItem) * hashTable->length);
    hashTable->size = 0;
    hashTable->hasFixedLength = hasFixedLength;
    hashTable->currentLoadFactor = 0;
    pthread_mutexattr_t mutex_attr;
    PLATFORM_initMutexAttr(&mutex_attr);
    PLATFORM_setMutexAttr_type(&mutex_attr, PTHREAD_MUTEX_ADAPTIVE_NP);
#ifdef MULTIAPP
    PLATFORM_setMutexAttr_pshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
#endif
    PLATFORM_initMutex(&(hashTable->mutex), &mutex_attr);

    /* Return the new table	*/
    return hashTable;
}

void xanHashTable_insert (XanHashTable *table, void *key, void *element) {
    unsigned int position;
    XanHashTableItem        *bucket;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Initialize the variables	*/
    position = -1;

    /* Check for free space */
    if (table->currentLoadFactor >= HASHTABLE_DEFAULT_LOAD_FACTOR) {

        /* We can't expand the table */
        if (table->hasFixedLength) {
            print_err("HASH TABLE: ERROR = The element cannot be stored in the hash table. ", 0);
            abort();
        }

        /* We can, so expand */
        _xanHashTableExpand(table);
    }

    /* Compute the position	of the	*
     * new element			*/
    position = _xanHashTableImproveHash(table, key);
    position = position % (table->length);
    bucket = &(table->table[position]);

    if (!bucket->used) {
        bucket->index = position;
        bucket->elementID = key;
        bucket->element = element;
        bucket->used = true;
        bucket->next = NULL;
        /* Update load factor   */
        table->currentLoadFactor += 1.f / table->length;

    } else {
        XanHashTableItem *item = table->alloc(sizeof(XanHashTableItem));
        item->index = position;
        item->next = NULL;
        item->elementID = key;
        item->element = element;
        item->used = true;
        while (bucket->next!=NULL) {
            bucket = bucket->next;
        }
        bucket->next = item;
    }

    table->size++;

    return;
}

void xanHashTable_replace (XanHashTable *table, void *key, void *element) {
    XanHashTableItem        *bucket;
    unsigned int position;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Compute the position	of the new element.
     */
    position 	= _xanHashTableImproveHash(table, key);
    position 	%= (table->length);
    bucket 		= &(table->table[position]);

    if (bucket->used) {
        if (table->equals(bucket->elementID,key)) {
            bucket->element = element;
        } else {
            bucket = bucket->next;
            while (bucket!=NULL) {
                if (table->equals(bucket->elementID,key)) {
                    bucket->element = element;
                    break;
                }
                bucket = bucket->next;
            }
        }
    }
}

void * xanHashTable_lookup (XanHashTable *table, void *key) {
    XanHashTableItem *bucket;
    unsigned int position;
    void*                   result;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    result = NULL;

    /* Compute the position	of the new element.
     */
    position 	= _xanHashTableImproveHash(table, key);
    position 	%= (table->length);
    bucket 		= &(table->table[position]);

    if (bucket->used) {
        if (table->equals(bucket->elementID,key)) {
            result = bucket->element;
        } else {
            bucket = bucket->next;
            while (bucket!=NULL) {
                if (table->equals(bucket->elementID,key)) {
                    result = bucket->element;
                    break;
                }
                bucket = bucket->next;
            }
        }
    }

    return result;
}

XanHashTableItem * xanHashTable_lookupItem (XanHashTable *table, void *key) {
    XanHashTableItem *bucket;
    unsigned int position;
    XanHashTableItem *                   result;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    result = NULL;

    /* Compute the position	of the	*
     * new element			*/
    position = _xanHashTableImproveHash(table, key);
    position %= (table->length);
    bucket = &(table->table[position]);

    if (bucket->used) {
        if (table->equals(bucket->elementID,key)) {
            result = bucket;
        } else {
            bucket = bucket->next;
            while (bucket!=NULL) {
                if (table->equals(bucket->elementID,key)) {
                    result = bucket;
                    break;
                }
                bucket = bucket->next;
            }
        }
    }
    return result;
}

void xanHashTable_addList (XanHashTable *table, XanList *listToAdd) {
    XanListItem	*item;

    if (listToAdd == NULL) {
        return ;
    }

    item	= xanList_first(listToAdd);
    while (item != NULL) {
        xanHashTable_insert(table, item->data, item->data);
        item	= item->next;
    }
}

XanHashTableItem* xanHashTable_first (XanHashTable* table) {
    unsigned int count;
    XanHashTableItem *bucket;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Initialize the variables	*/
    bucket = NULL;

    /* Delete the table		*/
    for (count = 0; count < table->length; count++) {
        if ((table->table[count]).used) {
            bucket = &(table->table[count]);
            break;
        }
    }

    /* Return			*/
    return bucket;
}

XanHashTableItem* xanHashTable_next (XanHashTable* table, XanHashTableItem* item) {
    XanHashTableItem *bucket;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);
    assert(item != NULL);

    /* Initialize the variables	*/
    bucket = NULL;

    if (item->next != NULL) {
        bucket = item->next;
    } else {
        unsigned int count;
        for (count = item->index+1; count < table->length; count++) {
            if ((table->table[count]).used) {
                bucket = &(table->table[count]);
                break;
            }
        }
    }

    return bucket;
}

int xanHashTable_elementsInside (XanHashTable *table) {
    int count;

    /* Assertions			*/
    assert(table != NULL);

    count = (int) table->size;

    return count;
}

void xanHashTable_removeItem (XanHashTable *table, XanHashTableItem *item) {
    XanHashTableItem	*bucket;
    XanHashTableItem	*prev;

    /* Assertions.
     */
    assert(table != NULL);
    assert(item != NULL);

    /* Check the item.
     */
    if (!item->used) {
        print_err("XANLIB: xanHashTableDelete: ERROR: Item to remove is not used. ", 0);
        abort();
    }
    assert(item->used);

    bucket 		= &(table->table[item->index]);
    if (item == bucket) {
        if (bucket->next != NULL) {
            XanHashTableItem *tempBucket;
            tempBucket = bucket->next;
            memcpy(bucket, tempBucket, sizeof(XanHashTableItem));
            table->free(tempBucket);

        } else {
            bucket->elementID = NULL;
            bucket->element = NULL;
            bucket->used = false;
            /* Update load factor   */
            table->currentLoadFactor -= 1.f / table->length;
        }
        table->size--;

        return ;
    }
    assert(item != bucket);
    prev = bucket;
    bucket = bucket->next;
    assert(bucket != NULL);
    while (bucket != NULL) {
        assert(prev != NULL);
        if (bucket == item) {
            prev->next = bucket->next;
            table->free(bucket);
            table->size--;
            return ;
        }
        prev = bucket;
        bucket = bucket->next;
    }

    return ;
}

void xanHashTable_removeElementKeyPair (XanHashTable *table, void *key, void *data) {
    XanHashTableItem* bucket;
    XanHashTableItem* prev;
    unsigned int position;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Initialize the variables	*/
    position = -1;
    prev = NULL;

    /* Compute the position	of the	*
     * new element			*/
    position = _xanHashTableImproveHash(table, key);
    position %= (table->length);
    bucket = &(table->table[position]);

    /* Check if the element exist	*/
    if (!bucket->used) {
        print_err("XANLIB: xanHashTableDelete: ERROR: Element not found. ", 0);
        abort();
    }

    if (	(table->equals(bucket->elementID, key))	&&
            (bucket->element == data)		) {
        if (bucket->next!=NULL) {
            XanHashTableItem *tempBucket;
            tempBucket = bucket->next;
            memcpy(bucket, tempBucket, sizeof(XanHashTableItem));
            table->free(tempBucket);

        } else {
            bucket->elementID = NULL;
            bucket->element = NULL;
            bucket->used = false;
            /* Update load factor   */
            table->currentLoadFactor -= 1.f / table->length;
        }
        table->size--;
        return ;
    }

    prev = bucket;
    bucket = bucket->next;
    assert(bucket != NULL);
    while (bucket != NULL) {
        assert(prev != NULL);
        if (	(table->equals(bucket->elementID, key))	&&
                (bucket->element == data)		) {
            prev->next = bucket->next;
            table->free(bucket);
            table->size--;
            return ;
        }
        prev = bucket;
        bucket = bucket->next;
    }

    print_err("XANLIB: xanHashTable_removeElement: ERROR: Element not found. ", 0);
    abort();
}

void xanHashTable_removeElements (XanHashTable *table, XanList *keysToRemove) {
    XanListItem *item;

    /* Assertions.
     */
    assert(table != NULL);
    assert(keysToRemove != NULL);

    item    = xanList_first(keysToRemove);
    while (item != NULL){
        xanHashTable_removeElement(table, item->data);
        item    = item->next;
    }

    return ;
}

void * xanHashTable_removeElement (XanHashTable *table, void *key) {
    XanHashTableItem* bucket;
    XanHashTableItem* prev;
    unsigned int position;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Initialize the variables	*/
    position = -1;
    prev = NULL;

    /* Compute the position	of the	*
     * new element			*/
    position = _xanHashTableImproveHash(table, key);
    position %= (table->length);
    bucket = &(table->table[position]);

    /* Check if the element exist	*/
    if (!bucket->used) {
        print_err("XANLIB: xanHashTableDelete: ERROR: Element not found. ", 0);
        abort();
    }

    if (table->equals(bucket->elementID, key)) {
        void	*oldKey;
        oldKey	= bucket->elementID;
        if (bucket->next!=NULL) {
            XanHashTableItem *tempBucket;
            tempBucket = bucket->next;
            memcpy(bucket, tempBucket, sizeof(XanHashTableItem));
            table->free(tempBucket);

        } else {
            bucket->elementID = NULL;
            bucket->element = NULL;
            bucket->used = false;
            /* Update load factor   */
            table->currentLoadFactor -= 1.f / table->length;
        }
        table->size--;
        return oldKey;
    }

    prev = bucket;
    bucket = bucket->next;
    assert(bucket != NULL);
    while (bucket != NULL) {
        assert(prev != NULL);
        if (table->equals(bucket->elementID, key)) {
            void	*oldKey;
            oldKey	= bucket->elementID;
            prev->next = bucket->next;
            table->free(bucket);
            table->size--;
            return oldKey;
        }
        prev = bucket;
        bucket = bucket->next;
    }

    print_err("XANLIB: xanHashTable_removeElement: ERROR: Element not found. ", 0);
    abort();
    return NULL;
}

void xanHashTable_deleteAndFreeElements (XanHashTable *table) {
    XanHashTableItem	*item;

    /* Assertions.
     */
    assert(table != NULL);

    /* Invoke the free function on elements.
     */
    item	= xanHashTable_first(table);
    while (item != NULL) {
        table->free(item->element);
        item	= xanHashTable_next(table, item);
    }

    /* Flush the table.
     */
    xanHashTable_emptyOutTable(table);

    return ;
}

void xanHashTable_emptyOutTableFreeKey (XanHashTable *table) {
    XanHashTableItem	*item;

    /* Assertions.
     */
    assert(table != NULL);

    /* Invoke the free function on elements.
     */
    item	= xanHashTable_first(table);
    while (item != NULL) {
        table->free(item->elementID);
        item	= xanHashTable_next(table, item);
    }

    /* Flush the table.
     */
    xanHashTable_emptyOutTable(table);

    return ;
}

void xanHashTable_emptyOutTableFreeDataAndKey (XanHashTable *table) {
    XanHashTableItem	*item;

    /* Assertions.
     */
    assert(table != NULL);

    /* Invoke the free function on elements.
     */
    item	= xanHashTable_first(table);
    while (item != NULL) {
        table->free(item->elementID);
        table->free(item->element);
        item	= xanHashTable_next(table, item);
    }

    /* Flush the table.
     */
    xanHashTable_emptyOutTable(table);

    return ;
}

void xanHashTable_emptyOutTable (XanHashTable *table) {
    unsigned int	c;

    if (table == NULL) {
        return ;
    }

    for (c=0; c < table->length; c++) {
        XanHashTableItem	*bucket;
        XanHashTableItem	*tempBucket;

        bucket	= &(table->table[c]);
        if (!bucket->used) {
            continue;
        }

        tempBucket		= bucket;
        assert(tempBucket != NULL);
        tempBucket		= tempBucket->next;
        while (tempBucket != NULL) {
            XanHashTableItem	*toDeleteBucket;
            toDeleteBucket		= tempBucket;
            tempBucket		= tempBucket->next;
            table->free(toDeleteBucket);
        }
        assert(bucket->index == c);
        bucket->used 		= false;
        bucket->elementID	= NULL;
        bucket->element	 	= NULL;
        bucket->next		= NULL;
    }
    table->currentLoadFactor	= 0;
    table->size			= 0;

    return ;
}

XanList * xanHashTable_toList (XanHashTable *table) {
    XanHashTableItem* bucket;
    XanList         *list;
    unsigned int count;

    /* Assertions			*/
    assert(table != NULL);

    /* Make a new list		*/
    list = xanList_new(table->alloc, table->free, NULL);
    assert(list != NULL);

    /* Fill the list		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            while (bucket!=NULL) {
                xanList_insert(list, bucket->element);
                bucket = bucket->next;
            }
        }
    }

    /* Return the list		*/
    return list;
}

XanList * xanHashTable_toKeyList (XanHashTable *table) {
    XanHashTableItem* bucket;
    XanList         *list;
    unsigned int count;

    /* Assertions			*/
    assert(table != NULL);

    /* Make a new list		*/
    list = xanList_new(table->alloc, table->free, NULL);
    assert(list != NULL);

    /* Fill the list		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            while (bucket!=NULL) {
                xanList_insert(list, bucket->elementID);
                bucket = bucket->next;
            }
        }
    }

    /* Return the list		*/
    return list;
}

XanList * xanHashTable_toSlotList (XanHashTable *table) {
    XanHashTableItem* bucket;
    XanList         *list;
    unsigned int count;

    /* Assertions			*/
    assert(table != NULL);

    /* Make a new list		*/
    list = xanList_new(table->alloc, table->free, NULL);
    assert(list != NULL);

    /* Fill the list		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            while (bucket!=NULL) {
                xanList_insert(list, &(bucket->element));
                bucket = bucket->next;
            }
        }
    }

    /* Return the list		*/
    return list;
}

XanList * xanHashTable_toItemList (XanHashTable *table) {
    XanHashTableItem* bucket;
    XanList         *list;
    unsigned int count;

    /* Assertions			*/
    assert(table != NULL);

    /* Make a new list		*/
    list = xanList_new(table->alloc, table->free, NULL);
    assert(list != NULL);

    /* Fill the list		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            while (bucket!=NULL) {
                xanList_insert(list, bucket);
                bucket = bucket->next;
            }
        }
    }

    /* Return the list		*/
    return list;
}

void ** xanHashTable_toKeyArray (XanHashTable *table) {
    XanHashTableItem* bucket;
    void ** array;
    unsigned int count;
    unsigned int index;

    /* Assertions			*/
    assert(table != NULL);

    /* Make a new array		*/
    array = table->alloc(sizeof(void *) * table->size);
    assert(array != NULL);

    /* Fill the array		*/
    index = 0;
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            while (bucket!=NULL) {
                assert(index < table->size);
                array[index] = bucket->elementID;
                bucket = bucket->next;
                index += 1;
            }
        }
    }

    /* Return the array		*/
    return array;
}

void ** xanHashTable_toDataArray (XanHashTable *table) {
    XanHashTableItem* bucket;
    void ** array;
    unsigned int count;
    unsigned int index;

    /* Assertions			*/
    assert(table != NULL);

    /* Make a new array		*/
    array = table->alloc(sizeof(void *) * table->size);
    assert(array != NULL);

    /* Fill the array		*/
    index = 0;
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            while (bucket!=NULL) {
                assert(index < table->size);
                array[index] = bucket->element;
                bucket = bucket->next;
                index += 1;
            }
        }
    }

    /* Return the array		*/
    return array;
}

XanHashTable * xanHashTable_cloneTable (XanHashTable *table) {
    XanHashTable    *newTable;
    void            *newElement;
    unsigned int count;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->alloc != NULL);
    assert(table->realloc != NULL);
    assert(table->free != NULL);
    assert(table->hash != NULL);
    assert(table->equals != NULL);

    /* Clone the table		*/
    newTable = xanHashTable_new(table->length, table->hasFixedLength, table->alloc, table->realloc, table->free, table->hash, table->equals);
    assert(newTable != NULL);
    xanHashTable_setCloneFunction(newTable, xanHashTable_getCloneFunction(table));
    assert(table->clone == newTable->clone);
    for (count = 0; count < table->length; count++) {
        XanHashTableItem *bucket = &(table->table[count]);
        if (bucket->used) {
            while (bucket!=NULL) {
                if (newTable->clone != NULL) {
                    assert(table->clone != NULL);
                    newElement = newTable->clone(bucket->element);
                } else {
                    assert(table->clone == NULL);
                    newElement = bucket->element;
                }
                xanHashTable_insert(newTable, bucket->elementID, newElement);
                bucket = bucket->next;
            }
        }
    }
    assert(xanHashTable_elementsInside(newTable) == xanHashTable_elementsInside(table));

    /* Return			*/
    return newTable;
}

void xanHashTable_destroyTable (XanHashTable *table) {
    unsigned int count;
    XanHashTableItem *bucket;
    XanHashTableItem *next;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Delete the table		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            bucket = bucket->next;
            while (bucket!=NULL) {
                next = bucket->next;
                table->free(bucket);
                bucket = next;
            }
        }
    }
    table->free(table->table);

    /* Delete the hash structure	*/
    table->free(table);

    /* Return			*/
    return;
}

void xanHashTable_destroyTableAndData (XanHashTable *table) {
    unsigned int count;
    XanHashTableItem *bucket;
    XanHashTableItem *next;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Delete the table		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            table->free(bucket->element);
            bucket = bucket->next;
            while (bucket!=NULL) {
                next = bucket->next;
                table->free(bucket->element);
                table->free(bucket);
                bucket = next;
            }
        }
    }
    table->free(table->table);

    /* Delete the hash structure	*/
    table->free(table);

    /* Return			*/
    return;
}

void xanHashTable_destroyTableAndKey (XanHashTable *table) {
    unsigned int count;
    XanHashTableItem *bucket;
    XanHashTableItem *next;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Delete the table		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            table->free(bucket->elementID);
            bucket = bucket->next;
            while (bucket!=NULL) {
                next = bucket->next;
                table->free(bucket->elementID);
                table->free(bucket);
                bucket = next;
            }
        }
    }
    table->free(table->table);

    /* Delete the hash structure	*/
    table->free(table);

    /* Return			*/
    return;
}

void xanHashTable_destroyTableDataAndKey (XanHashTable *table) {
    unsigned int count;
    XanHashTableItem *bucket;
    XanHashTableItem *next;

    /* Assertions			*/
    assert(table != NULL);
    assert(table->length > 0);
    assert(table->table != NULL);

    /* Delete the table		*/
    for (count = 0; count < table->length; count++) {
        bucket = &(table->table[count]);
        if (bucket->used) {
            table->free(bucket->element);
            table->free(bucket->elementID);
            bucket = bucket->next;
            while (bucket!=NULL) {
                next = bucket->next;
                table->free(bucket->element);
                table->free(bucket->elementID);
                table->free(bucket);
                bucket = next;
            }
        }
    }
    table->free(table->table);

    /* Delete the hash structure	*/
    table->free(table);

    /* Return			*/
    return;
}

void xanHashTable_setCloneFunction (XanHashTable *table, void * (*cloneFunction)(void *data)) {

    /* Assertions			*/
    assert(table != NULL);

    table->clone = cloneFunction;

    return ;
}

void * xanHashTable_getCloneFunction (XanHashTable *table) {

    /* Assertions			*/
    assert(table != NULL);

    return table->clone;
}

void xanHashTable_lock (XanHashTable *table) {

    /* Assertions			*/
    assert(table != NULL);
    PLATFORM_lockMutex(&(table->mutex));
}

void xanHashTable_unlock (XanHashTable *table) {

    /* Assertions			*/
    assert(table != NULL);
    PLATFORM_unlockMutex(&(table->mutex));
}

void xanHashTable_syncInsert (XanHashTable *table, void *key, void *element) {

    /* Assertions			*/
    assert(table != NULL);
    assert(key != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    xanHashTable_insert(table, key, element);
    PLATFORM_unlockMutex(&(table->mutex));
}

void * xanHashTable_syncLookup (XanHashTable *table, void *key) {
    void    *data;

    /* Assertions			*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    data = xanHashTable_lookup(table, key);
    PLATFORM_unlockMutex(&(table->mutex));

    return data;
}

XanHashTableItem * xanHashTable_syncFirst (XanHashTable* table) {
    XanHashTableItem *bucket;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    bucket = xanHashTable_first(table);
    PLATFORM_unlockMutex(&(table->mutex));

    return bucket;
}

XanHashTableItem * xanHashTable_syncNext (XanHashTable* table, XanHashTableItem* item) {
    XanHashTableItem *bucket;

    /* Assertions		*/
    assert(table != NULL);
    assert(item != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    bucket = xanHashTable_next(table, item);
    PLATFORM_unlockMutex(&(table->mutex));

    return bucket;
}

int xanHashTable_syncElementsInside (XanHashTable* table) {
    int numElements;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    numElements = xanHashTable_elementsInside(table);
    PLATFORM_unlockMutex(&(table->mutex));

    return numElements;
}

void * xanHashTable_syncRemoveElement (XanHashTable *table, void *key) {
    void	*oldKey;

    /* Assertions			*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    oldKey	= xanHashTable_removeElement(table, key);
    PLATFORM_unlockMutex(&(table->mutex));

    return oldKey;
}

void xanHashTable_syncEmptyOutTable (XanHashTable *table) {

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    xanHashTable_emptyOutTable(table);
    PLATFORM_unlockMutex(&(table->mutex));
}

XanList * xanHashTable_syncToList (XanHashTable *table) {
    XanList *list;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    list = xanHashTable_toList(table);
    PLATFORM_unlockMutex(&(table->mutex));

    return list;
}

XanList * xanHashTable_syncToKeyList (XanHashTable *table) {
    XanList *list;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    list = xanHashTable_toKeyList(table);
    PLATFORM_unlockMutex(&(table->mutex));

    return list;
}

XanList * xanHashTable_syncToSlotList (XanHashTable *table) {
    XanList *list;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    list = xanHashTable_toSlotList(table);
    PLATFORM_unlockMutex(&(table->mutex));

    return list;
}

XanList * xanHashTable_syncToItemList (XanHashTable *table) {
    XanList *list;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    list = xanHashTable_toItemList(table);
    PLATFORM_unlockMutex(&(table->mutex));

    return list;
}

XanHashTable * xanHashTable_syncCloneTable (XanHashTable *table) {
    XanHashTable    *newTable;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    newTable = xanHashTable_cloneTable(table);
    assert(newTable != NULL);
    PLATFORM_unlockMutex(&(table->mutex));

    return newTable;
}

void * xanHashTable_syncGetCloneFunction (XanHashTable *table) {
    void    *function;

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    function = xanHashTable_getCloneFunction(table);
    PLATFORM_unlockMutex(&(table->mutex));

    return function;
}

void xanHashTable_syncSetCloneFunction (XanHashTable *table, void * (*cloneFunction)(void *data)) {

    /* Assertions		*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    xanHashTable_setCloneFunction(table, cloneFunction);
    PLATFORM_unlockMutex(&(table->mutex));

    return ;
}

void xanHashTable_syncDestroyTable (XanHashTable *table) {

    /* Assertions			*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    xanHashTable_destroyTable(table);
}

void xanHashTable_syncDestroyTableAndData (XanHashTable *table) {

    /* Assertions			*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    xanHashTable_destroyTableAndData(table);
}

void xanHashTable_syncDestroyTableAndKey (XanHashTable *table) {

    /* Assertions			*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    xanHashTable_destroyTableAndKey(table);
}

void xanHashTable_syncDestroyTableDataAndKey (XanHashTable *table) {

    /* Assertions			*/
    assert(table != NULL);

    PLATFORM_lockMutex(&(table->mutex));
    xanHashTable_destroyTableDataAndKey(table);
}
