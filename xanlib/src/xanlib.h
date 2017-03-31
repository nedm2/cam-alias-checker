/*
 * Copyright (C)  2012 Simone Campanoni, Timothy M. Jones
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
/**
 * @file xanlib.h Structure of the XanLib
 * @brief Structure of the XanLib
 */
#ifndef XANLIB_H
#define XANLIB_H

#include <platform_API.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @mainpage XanLib
 *
 * \author Simone Campanoni       http://campanoni.users.sourceforge.net
 * \version 0.3.0
 *
 * Project web page: \n
 *      http://xanlib.sourceforge.net
 *
 * The XanLib library provides structures to manage generic data.
 *
 * Each data structure provides functionality in two forms: synchronized (thread-safe) and unsynchronized (thread-unsafe).
 *
 * In the synchronized version of the method, the library, before performing the method's action, takes the lock to the structure to guarantee thread safety.
 *
 * The following page gives instructions and examples on the usage of this library: \ref XanLib
 *
 * For the functions to use to allocate new structures, please refer to the <code>xanlib.h</code> file.
 */

/**
 * \defgroup XanList List
 * \ingroup XanLibDoc
 *
 * In order to allocate a list (XanList) we have to write the following lines of code:\n
 * @code
 *      XanList *myList;
 *      myList  = xanList_new(malloc, free, NULL);
 * @endcode
 * \n\n
 * In order to insert an element into a list for a single thread program, we have to write the following lines of code:\n
 * @code
 *      xanList_insert(myList, myNewElementToInsert);
 * @endcode
 * \n\n
 * In order to delete the list without deleting the elements collected inside, we have to write the following line of code:\n
 * @code
 *      xanList_destroyList(myList);
 * @endcode
 * \n\n
 * In the sequel, we provide the lines of code that iterates among a list.\n
 * @code
 *      XanListItem	*item;
 *	myType		*myData;
 *	item	= xanList_first(myList);
 *      while (item != NULL){
 *		myData	= (myType *) item->data;
 *		item	= item->next;
 *	}
 * @endcode
 *
 * Next: \ref XanVar
 */

/**
 * \defgroup XanListAllocate Allocate a new list
 * \ingroup XanList
 */

/**
 * \defgroup XanListAdd Adding new elements to a list
 * \ingroup XanList
 */

/**
 * \defgroup XanListRemove Removing elements from a list
 * \ingroup XanList
 */

/**
 * \defgroup XanListConversion Converting a list
 * \ingroup XanList
 */

/**
 * \defgroup XanListDestroy Destroy a list
 * \ingroup XanList
 */

/**
 * @defgroup XanVar Variable
 * @ingroup XanLibDoc
 *
 * Next: \ref XanPipe
 */

/**
 * @defgroup XanPipe Pipe
 * @ingroup XanLibDoc
 *
 * Next: \ref XanQueue
 */

/**
 * @defgroup XanQueue Priority Queue
 * @ingroup XanLibDoc
 *
 * Next: \ref XanStack
 */

/**
 * @defgroup XanStack Stack
 * @ingroup XanLibDoc
 *
 * In order to allocate a stack, we have to write the following:\n
 * @code
 *      XanStack *myStack;
 *      myStack = xanStack_new(malloc, free, NULL);
 * @endcode
 * \n\n
 * In order to push @c element onto a stack, we have to write:\n
 * @code
 *      xanStack_push(myStack, element);
 * @endcode
 * \n\n
 * In order to view the top of the stack, without altering the stack, we have to write:\n
 * @code
 *      myType *myData;
 *      myData = (myType *) xanStack_top(myStack);
 * @endcode
 * \n\n
 * In order to deallocate a stack, without destroying the items on it, we have to write:\n
 * @code
 *      xanStack_destroyStack(myStack);
 * @endcode
 *
 * Next: \ref XanNode
 */

/**
 * @defgroup XanNode Tree
 * @ingroup XanLibDoc
 *
 * Next: \ref XanHashTable
 */

/**
 * @defgroup XanHashTable Hash Table
 * @ingroup XanLibDoc
 *
 * In order to allocate a hash table, initially holding 11 items, with the default hash and compare functions, we have to write the following:\n
 * @code
 *      XanHashTable *myTable;
 *      myTable = xanHashTable_new(11, 0, malloc, realloc, free, NULL, NULL);
 * @endcode
 * \n\n
 * In order to add @c element to the hash table with key @c key, we have to write the following:\n
 * @code
 *      xanHashTable_insert(myTable, key, element);
 * @endcode
 * \n\n
 * In order to deallocate a hash table without deleting the elements inside, we have to write:\n
 * @code
 *      xanHashTable_destroyTable(myTable);
 * @endcode
 * \n\n
 * In order to iterate over the contents of a hash table, we can do the following:\n
 * @code
 *      XanHashTableItem *item;
 *      myKeyType        *myKey;
 *      myDataType       *myData;
 *      item = xanHashTable_first(myTable);
 *      while (item) {
 *           myKey  = (myKeyType *) item->elementID;
 *           myData = (myDataType *) item->element;
 *           item   = xanHashTable_next(myTable, item);
 *      }
 * @endcode
 *
 * Next: \ref XanGraph
 */

/**
 * \defgroup XanHashTableAllocate Allocate a new hash table
 * \ingroup XanHashTable
 */

/**
 * \defgroup XanHashTableAdd Adding new elements to a hash table
 * \ingroup XanHashTable
 */

/**
 * \defgroup XanHashTableRemove Removing elements from a hash table
 * \ingroup XanHashTable
 */

/**
 * \defgroup XanHashTableConversion Converting a hash table
 * \ingroup XanHashTable
 */

/**
 * \defgroup XanHashTableDestroy Destroy a hash table
 * \ingroup XanHashTable
 */

/**
 * @defgroup XanGraph Graph
 * @ingroup XanLibDoc
 *
 * Next: \ref XanBitSet
 */

/**
 * @defgroup XanBitSet Bitset
 * @ingroup XanLibDoc
 *
 * In order to allocate a bitset of length @c length we have to write the following lines of code:\n
 * @code
 *      XanBitSet *mySet;
 *      mySet  = xanBitSet_new(length);
 * @endcode
 * \n\n
 * In order to set bit @c n, we have to write the following lines of code:\n
 * @code
 *      xanBitSet_setBit(mySet, n);
 * @endcode
 * \n\n
 * In order to check whether a bit @c n is set, we have to write:\n
 * @code
 *      bool isSet;
 *      isSet = xanBitSet_isBitSet(mySet, n);
 * @endcode
 * \n\n
 * In order to deallocate the bitset, we have to write:\n
 * @code
 *      xanBitSet_free(mySet);
 * @endcode
 *
 * Next: \ref Copyright
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * #     #                 #
 *  #   #     ##    #    # #           #     ####    #####
 *   # #     #  #   ##   # #           #    #          #
 *    #     #    #  # #  # #           #     ####      #
 *   # #    ######  #  # # #           #         #     #
 *  #   #   #    #  #   ## #           #    #    #     #
 * #     #  #    #  #    # #######     #     ####      #
 **/

/**
 * @brief List element
 * \ingroup XanList
 *
 * Element stored inside the XanList list
 */
typedef struct XanListItem {
    void                    * data;         /**< Pointer to the data which belongs to the item of the list	*/
    struct XanListItem      * next;         /**< Next element of the list					*/
    struct XanListItem      * prev;         /**< Previous element of the list				*/
} XanListItem;

/**
 * @brief Doubly linked list
 * \ingroup XanList
 *
 * Doubly linked list containing pointers to user data with the ability to iterate over the list in both directions.
 */
typedef struct XanList {
    void *                  (*alloc)(size_t size);                  /**< Alloc function of the list	*/
    void 			        (*free)	(void *address);                /**< Free function of the list	*/
    void *                  (*clone)(void *data);                   /**< Clone function of the list. It clones the data field stored addressed by the elements inside the list	*/
    int 			        len;
    XanListItem             **bufferItems;
    JITUINT32               bufferItemsTop;
    JITUINT32               bufferItemsSize;
    XanListItem             *firstItem;
    XanListItem             *lastItem;
    pthread_mutex_t 	    itemsMutex;
} XanList;

/**
 * @brief Element of the hash table.
 * \ingroup XanHashTable
 */
typedef struct XanHashTableItem {
    bool 			used;
    unsigned int 		index;
    void    		*elementID;
    void    		*element;
    struct XanHashTableItem *next;
} XanHashTableItem;

/**
 * @brief Hash table
 * \ingroup XanHashTable
 *
 * This structure implements the closed hash table. The equals, hash functions can be customized by the user.
 */
typedef struct XanHashTable {
    XanHashTableItem        *table;
    unsigned int 		primeIndex;
    unsigned int 		length;
    unsigned int 		size;
    int 			hasFixedLength;
    float 			currentLoadFactor;
    pthread_mutex_t 	mutex;

    void *                  (*alloc)(size_t size);
    void *                  (*realloc)(void *oldAddress, size_t newSize);
    void 			(*free)(void *addr);
    unsigned int 		(*hash)(void *element);
    int 			(*equals)(void *key1, void *key2);
    void *                  (*clone)(void *data);
} XanHashTable;

/**
 * \ingroup XanListAllocate
 * @brief Make a new list.
 *
 * Make a new list.
 * e.g. XanList *myList = xanListNew(malloc, free, NULL);
 *
 * @param allocFunction This function is called each time the list needs to allocate new memory; if it is NULL, then the list will use the malloc function.
 * @param freeFunction This function is called each time the list needs to free memory; if it is NULL, then the list will use the free function.
 * @param cloneFunction This function is called each time the list needs to clone the variable stored inside its elements; if it is NULL, then the list does not clone the variable stored inside each element of the list.
 * @result Return the new list.
 */
XanList * xanList_new(void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void * (*cloneFunction)(void *addr));

/**
 * @brief Get the first item within a list.
 * \ingroup XanList
 *
 * Return the head item in @c list.
 *
 * @param list The list to consider.
 * @return The first item @c list, or @c NULL if @c list is empty.
 */
XanListItem * xanList_first (XanList *list);

/**
 * @brief Get the last item within a list.
 * \ingroup XanList
 *
 * Return the last item in @c list.
 *
 * @param list The list to consider
 * @return The last item in @c list, or @c NULL if @c list is empty.
 */
XanListItem * xanList_last (XanList *list);

/**
 * @brief Get the next item from a list.
 * @ingroup XanList
 *
 * Return the item following @c item in its list.
 *
 * @param item The list item to consider.
 * @return The item after @c item within its list, or @c NULL if @c item is the last in the list.
 */
XanListItem * xanList_next (XanListItem *item);

/**
 * \ingroup XanList
 * @brief Get the previous item from a list.
 *
 * Return the item preceeding @c item in its list.
 *
 * @param item The list item to consider.
 * @return The item before @c item within its list, or @c NULL if @c item is the first in the list.
 */
XanListItem * xanList_prev (XanListItem *item);

/**
 * \ingroup XanListConversion
 * @brief Convert a list to a table
 *
 * Convert the list @c l to a hash table.
 *
 * For each element of the list (i.e., @c void *), an equivalent element in the hash table is created; the element and its ID is the @c void * of the list element.
 *
 * The caller is in charge to free the returned table whenever he or she does not need it anymore.
 *
 * For example:
 * @code
 * XanList *l;
 * XanHashTable *t;
 *
 * // Allocate a list
 * l = xanList_new(malloc, free, NULL);
 * xanList_append(l, mydata);
 *
 * // Convert the just-allocated list
 * t = xanList_toHashTable(l);
 *
 * // Use the hash table
 * codeThatUsesTable(t);
 *
 * // Free the memory
 * xanHashTable_destroyTable(t);
 * xanList_destroyList(l);
 * @endcode
 *
 * @param l The list to convert
 * @return An hash table containing all the elements of @c l .
 */
XanHashTable * xanList_toHashTable (XanList *l);

/**
 * \ingroup XanListAdd
 * @brief Append a data element to the end of a list.
 *
 * Append @c data to the end of @c list.
 *
 * @param list The list to append to.
 * @param data The data to append.
 * @return The list item that stores @c data within @c list.
 */
XanListItem * xanList_append (XanList *list, void *data);

/**
 * \ingroup XanListAdd
 * @brief Append the contents of one list to the end of another.
 *
 * Append all data elements stored in @c fromList to the end of @c toList.
 *
 * The list @c fromList is unchanged.
 *
 * The same order of elements that belong to @c fromList will be preserved in @c toList .
 *
 * @param toList The list to append to.
 * @param fromList The list to append from.
 */
void xanList_appendList (XanList *toList, XanList *fromList);

/**
 * \ingroup XanListAdd
 * @brief Insert some data at the start of a list.
 *
 * Insert @c data at the start of @c list.
 *
 * @param list The list to insert into.
 * @param data The data to insert.
 * @return The list item that stores @c data in @c list.
 */
XanListItem * xanList_insert (XanList *list, void *data);

/**
 * \ingroup XanListAdd
 * @brief Insert some data into a list before an existing item.
 *
 * Insert @c data into @c list before the existing list item @c next.
 *
 * @param list The list to insert into.
 * @param next The existing list item to insert before.
 * @param data The data to insert.
 * @return The list item that stores @c data in @c list.
 */
XanListItem * xanList_insertBefore (XanList *list, XanListItem *next, void *data);

/**
 * \ingroup XanListAdd
 * @brief Insert some data into a list after an existing item.
 *
 * Insert @c data into @c list after the existing list item @c prev.
 *
 * @param list The list to insert into.
 * @param prev The existing list item to insert after.
 * @param data The data to insert.
 * @return The list element that stores @c data in @c list.
 */
XanListItem * xanList_insertAfter (XanList *list, XanListItem *prev, void *data);

/**
 * @brief Move a list item to the beginning of the list.
 * \ingroup XanList
 *
 * Move @c item to the beginning of @c list.
 *
 * @param list The list to move in.
 * @param item The item to move.
 * @return The new first item in @c list.
 */
XanListItem * xanList_moveToBegin (XanList *list, XanListItem *item);

/**
 * @brief Move a list item just before another one.
 * \ingroup XanList
 *
 * Move @c itemToMove just before @c place in the list @c list .
 *
 * @param list List that contains both @c itemToMove and @c place
 * @param itemToMove The item to move.
 * @param place @c itemToMove is moved just before this.
 */
void xanList_moveBeforeItem (XanList *list, XanListItem *itemToMove, XanListItem *place);

/**
 * @brief Find the list item that stores some data within a list.
 * \ingroup XanList
 *
 * Get the list item that stores @c data within @c list, if there is one.
 *
 * @param list The list to search in.
 * @param data The data to search for.
 * @return The list item that stores @c data within @c list, or @c NULL if there isn't one.
 */
XanListItem * xanList_find (XanList *list, void *data);

/**
 * @brief Get the data element stored in a list item.
 * @ingroup XanList
 *
 * Get the data element stored in @c item.
 *
 * @param item The list item that stores the data.
 * @return The data element stored in @c item.
 */
void * xanList_getData (XanListItem *item);

/**
 * @brief Get the number of data elements within a list.
 * \ingroup XanList
 *
 * Get the number of data elements stored in @c list.
 *
 * @param list The list to consider.
 * @return The number of data elements in @c list.
 */
int xanList_length (XanList *list);

/**
 * @brief Get the position of a data element within a list.
 * \ingroup XanList
 *
 * Return the position of @c data within @c list.
 *
 * @param list The list to look in.
 * @param data The data to look for.
 * @return The position of @c data in @c list (starting at 0), or -1 if @c data is not in @c list.
 */
int xanList_getPositionNumberFromElement (XanList *list, void *data);

/**
 * @brief Get the list item from a list in a given position.
 * \ingroup XanList
 *
 * Get the list item from @c list that is in position @c positionNumber.
 *
 * @param list The list to look in.
 * @param positionNumber The position of the item to get.
 * @return The list item at position @c positionNumber.
 */
XanListItem * xanList_getElementFromPositionNumber (XanList *list, int positionNumber);

/**
 * @brief Check whether two lists have at least one data element in common.
 * \ingroup XanList
 *
 * Check whether @c list and @c otherList store at least one data element that is the same.
 *
 * @param list The first list to consider.
 * @param otherList The second list to consider.
 * @return @c true if there is at least one shared element, @c false otherwise.
 */
bool xanList_shareSomeElements (XanList *list, XanList *otherList);

/**
 * @brief Check whether two lists contain exactly the same data elements.
 * \ingroup XanList
 *
 * Check whether @c list and @c otherList store exactly the same data elements, with no extras in either.
 *
 * @param list The first list to consider.
 * @param otherList The second list to consider.
 * @return @c true if both lists contain exactly the same elements, @c false otherwise.
 */
bool xanList_containsTheSameElements (XanList *list, XanList *otherList);

/**
 * @brief Check whether a list's data elements are all in another list too.
 * @ingroup XanList
 *
 * Check whether all elements in @c subList are also in @c superList.  In other words, @c superList is a superset of @c subList.
 *
 * @param subList The list containing data elements to check.
 * @param superList The list to check for data elements in.
 * @return @c true if all data elements in @c subList are also in @c superList, @c false otherwise.
 */
bool xanList_containsAllElements (XanList *subList, XanList *superList);

/**
 * @brief Get the number of instances of a data element within a list.
 * \ingroup XanList
 *
 * Count the number of times @c data occurs within @c list.
 *
 * @param list The list to look in.
 * @param data The data element to search for.
 * @return The number of times @c data is found in @c list.
 */
int xanList_equalsInstancesNumber (XanList *list, void *data);

/**
 * @brief Get a pointer to the data element stored inside a given list item.
 * \ingroup XanList
 *
 * Get a pointer to the user data stored within the list item @c item.  The stored data is already a pointer, so this returns a pointer to a pointer.
 *
 * @param item The list item containing the data.
 * @return A pointer to the data stored within @c item.
 */
void ** xanList_getSlotData (XanListItem *item);

/**
 * @brief Get a new list containing data that is common between two lists.
 * \ingroup XanList
 *
 * Get a new list that stores data found in both @c list1 and @c list2. If there is no common data then an empty (but not @c NULL) list will be returned.
 *
 * @param list1 The first list to consider.
 * @param list2 The second list to consider.
 * @return A new list containing data shared by @c list1 and @c list2.
 */
XanList *xanList_getSharedElements(XanList *list1, XanList *list2);

/**
 * \ingroup XanListRemove
 * @brief Remove all data elements from a list.
 *
 * Remove all data elements from @c list. The data is not removed and this simply leaves an empty list.
 *
 * @param list The list to empty.
 */
void xanList_emptyOutList (XanList *list);

/**
 * \ingroup XanListDestroy
 * @brief Destroy a list
 *
 * Destroy @c list by calling its free function.  This does not destroy the data stored in @c list.
 *
 * @param list The list to destroy.
 */
void xanList_destroyList (XanList *list);

/**
 * \ingroup XanListDestroy
 * @brief Destroy a list and its data elements
 *
 * Destroy @c list and each data element stored inside it.
 *
 * @param list The list to destroy.
 */
void xanList_destroyListAndData (XanList *list);

/**
 * \ingroup XanListRemove
 * @brief Delete a list item from a list.
 *
 * Remove @c item from @c list. The data that @c item stores is not deleted.
 *
 * @param list The list to remove from.
 * @param item The list item to delete.
 */
void xanList_deleteItem (XanList *list, XanListItem *item);

/**
 * \ingroup XanListRemove
 * @brief Remove a data element from a list.
 *
 * Remove the list item from @c list that stores @c data.  This does not delete @c data.  If more than one item stores @c data then only the first item is deleted (as found by @c xanList_find).
 *
 * @param list The list to remove from.
 * @param data The data stored by the item to remove.
 * @param abortIfAnElementDoNotExist JITTRUE or JITFALSE
 */
void xanList_removeElement (XanList *list, void *data, JITBOOLEAN abortIfAnElementDoNotExist);

/**
 * \ingroup XanListRemove
 * @brief Remove elements from a list.
 *
 * Remove all elements included in @c elementsToRemove from @c list that stores them.
 *
 * This does not delete the @c data itself.
 *
 * If more than one item stores @c data then only the first item is deleted (as found by @c xanList_find).
 *
 * @param list The list to remove from.
 * @param elementsToRemove Elements to remove.
 * @param abortIfAnElementDoNotExist JITTRUE or JITFALSE
 */
void xanList_removeElements (XanList *list, XanList *elementsToRemove, JITBOOLEAN abortIfAnElementDoNotExist);

/**
 * \ingroup XanListRemove
 * @brief Remove elements from a list.
 *
 * Remove all elements included in @c elementsToRemove from @c list that stores them.
 *
 * This does not delete the @c data itself.
 *
 * All items in @c list that store @c data included in @c elementsToRemove are removed.
 *
 * Each key of @c elementsToRemove are the data stored by items removed from @c list.
 *
 * @param list The list to remove from.
 * @param elementsToRemove Elements to remove.
 * @param abortIfAnElementDoNotExist JITTRUE or JITFALSE
 */
void xanList_removeElementsContainedInTable (XanList *list, XanHashTable *elementsToRemove, JITBOOLEAN abortIfAnElementDoNotExist);

/**
 * \ingroup XanListRemove
 * @brief Remove all list items containing some data from a list.
 *
 * Remove all items from @c list that store @c data.  This does not delete @c data.
 *
 * @param list The list to remove from
 * @param data The data stored by the items to remove.
 */
void xanList_removeAllElements (XanList *list, void *data);

/**
 * \ingroup XanListRemove
 * @brief Empty out a list and free all data elements.
 *
 * Empty out @c list and free the memory used by all data elements contained in it.
 *
 * @param list The list to delete.
 */
void xanList_deleteAndFreeElements (XanList *list);

/**
 * \ingroup XanListRemove
 * @brief Remove duplicate data elements from a list.
 *
 * Remove list items from @c list that store the same data as each other. Afterwards @c list will contain only one list item for each data element stored.
 *
 * @param list The list to consider.
 */
void xanList_deleteClones (XanList *list);

/**
 * @brief Set the clone function for a list.
 * \ingroup XanList
 *
 * Set the clone function of @c list to be @c cloneFunction. The clone function is used to clone data items when cloning a whole list. It can be @c NULL.
 *
 * @param list The list to consider.
 * @param cloneFunction The new clone function to use for @c list.
 */
void xanList_setCloneFunction (XanList *list, void * (*cloneFunction)(void *data));

/**
 * @brief Get the clone function for a list.
 * \ingroup XanList
 *
 * Get the clone function that is used for @c list.
 *
 * @param list The list to consider.
 * @return The clone function for @c list.
 */
void * xanList_getCloneFunction (XanList *list);

/**
 * \ingroup XanListAllocate
 * @brief Clone a whole list.
 *
 * Clone @c list.  Clone all data items that are held in @c list too iff
 * the clone function for the list is defined (for example, through a
 * call to @c xanList_setCloneFunction).
 *
 * @param list The list to clone.
 * @return A clone of @c list.
 */
XanList * xanList_cloneList (XanList *list);

/**
 * @ingroup XanListAllocate
 * @brief Reverse a list.
 *
 * Reverse @c list in place or as a new list if @c clone is @c true.
 *
 * @param list The list to reverse.
 * @param clone Reverse a clone of @c list if @c true, by first calling @c xanList_cloneList.
 * @return The reversed list (which will be a new list if @c clone is @c true).
 */
XanList * xanList_reverseList (XanList *list, bool clone);

/**
 * @brief Lock a list.
 * @ingroup XanList
 *
 * Lock the mutex associated with @c list.
 *
 * @param list The list to lock.
 */
void xanList_lock (XanList *list);

/**
 * @brief Unlock a list.
 * @ingroup XanList
 *
 * Unlock the mutex associated with @c list.
 *
 * @param list The list to unlock.
 */
void xanList_unlock (XanList *list);

/**
 * \ingroup XanListAdd
 * @brief Append a data element to the end of a list, thread-safe.
 *
 * Append @c data to the end of @c list.
 *
 * @param list The list to append to.
 * @param data The data to append.
 * @return The list item that stores @c data within @c list.
 */
XanListItem * xanList_syncAppend (XanList *list, void *data);

/**
 * \ingroup XanListAdd
 * @brief Append the contents of one list to the end of another, thread-safe.
 *
 * Append all data elements stored in @c fromList to the end of @c toList.  The list @c fromList is unchanged.
 *
 * @param toList The list to append to.
 * @param fromList The list to append from.
 */
void xanList_syncAppendList (XanList *toList, XanList *fromList);

/**
 * \ingroup XanListAdd
 * @brief Insert some data at the start of a list, thread-safe.
 *
 * Insert @c data at the start of @c list.
 *
 * @param list The list to insert into.
 * @param data The data to insert.
 * @return The list item that stores @c data in @c list.
 */
XanListItem * xanList_syncInsert (XanList *list, void *data);

/**
 * @brief Find the list item that stores some data within a list, thread-safe.
 * \ingroup XanList
 *
 * Get the list item that stores @c data within @c list, if there is one.
 *
 * @param list The list to search in.
 * @param data The data to search for.
 * @return The list item that stores @c data within @c list, or @c NULL if there isn't one.
 */
XanListItem * xanList_syncFind (XanList *list, void *data);

/**
 * \ingroup XanListRemove
 * @brief Remove a data element from a list, thread-safe.
 *
 * Remove the list item from @c list that stores @c data.  This does not delete @c data.  If more than one item stores @c data then only the first item is deleted (as found by @c xanList_find).
 *
 * @param list The list to remove from.
 * @param data The data stored by the item to remove.
 */
void xanList_syncRemoveElement (XanList *list, void *data);

/**
 * @brief Reverse a list, thread-safe.
 * @ingroup XanList
 *
 * Reverse @c list in place or as a new list if @c clone is @c true.
 *
 * @param list The list to reverse.
 * @param clone Reverse a clone of @c list if @c true, by first calling @c xanList_cloneList.
 * @return The reversed list (which will be a new list if @c clone is @c true).
 */
XanList * xanList_syncReverseList (XanList *list, bool clone);

/**
 * #     #                 #     #
 *  #   #     ##    #    # #     #    ##    #####
 *   # #     #  #   ##   # #     #   #  #   #    #
 *    #     #    #  # #  # #     #  #    #  #    #
 *   # #    ######  #  # #  #   #   ######  #####
 *  #   #   #    #  #   ##   # #    #    #  #   #
 * #     #  #    #  #    #    #     #    #  #    #
 **/

/**
 * @brief Variable
 * \ingroup XanVar
 *
 * Variable which store pointer
 */
typedef struct XanVar {
    void    *data;
    pthread_mutex_t mutex;
    void *                  (*alloc)(size_t size);
    void (*free)(void *address);
} XanVar;


/**
 * @brief Make a new variable.
 * \ingroup XanVar
 *
 * Make a new variable.
 *
 * @param allocFunction This function is called each time the variable needs to allocate new memory; if it is NULL, then the variable will use the malloc function.
 * @param freeFunction This function is called each time the variable needs to free memory; if it is NULL, then the variable will use the free function.
 * @result Return the new variable.
 */
XanVar * xanVar_new(void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr));

/**
 * @brief Read a variable.
 * @ingroup XanVar
 *
 * Read the data stored in @c var.
 *
 * @param var The variable to read from.
 * @return The data stored in @c var.
 */
void * xanVar_read(XanVar *var);

/**
 * @brief Write a variable.
 * @ingroup XanVar
 *
 * Write to the data in @c var.
 *
 * @param var The variable to write to.
 * @param data The data to write.
 */
void xanVar_write(XanVar *var, void *data);

/**
 * @brief Destroy a variable.
 * @ingroup XanVar
 *
 * Destroy @c var by calling its free function.  This does not destroy the data stored in @c var.
 *
 * @param var The variable to destroy.
 */
void xanVar_destroyVar(XanVar *var);

/**
 * @brief Destroy a variable and its data element.
 * @ingroup XanVar
 *
 * Destroy @c var and the data element stored inside it by calling the variable's free function on both structures.
 *
 * @param var The variable to destroy.
 */
void xanVar_destroyVarAndData(XanVar *var);

/**
 * @brief Lock a variable.
 * @ingroup XanVar
 *
 * Lock the mutex associated with @c var.
 *
 * @param var The variable to lock.
 */
void xanVar_lock(XanVar *var);

/**
 * @brief Unlock a variable.
 * @ingroup XanVar
 *
 * Unlock the mutex associated with @c var.
 *
 * @param var The variable to unlock.
 */
void xanVar_unlock(XanVar *var);

/**
 * @brief Read a variable, thread-safe.
 * @ingroup XanVar
 *
 * Read the data stored in @c var.
 *
 * @param var The variable to read from.
 * @return The data stored in @c var.
 */
void * xanVar_syncRead(XanVar *var);

/**
 * @brief Write a variable, thread-safe.
 * @ingroup XanVar
 *
 * Write to the data in @c var.
 *
 * @param var The variable to write to.
 * @param data The data to write.
 */
void xanVar_syncWrite(XanVar *var, void *data);


/**
 * #     #                 ######
 *  #   #     ##    #    # #     #     #    #####   ######
 *   # #     #  #   ##   # #     #     #    #    #  #
 *    #     #    #  # #  # ######      #    #    #  #####
 *   # #    ######  #  # # #           #    #####   #
 *  #   #   #    #  #   ## #           #    #       #
 * #     #  #    #  #    # #           #    #       ######
 **/

/**
 * @brief Pipe
 * \ingroup XanPipe
 *
 * This is a pipe structure, which can store inside itself unbounded elements.
 */
typedef struct XanPipe {
    void *          (*alloc)(size_t size);
    void (*free)(void *addr);
    XanList         *items;
    pthread_mutex_t itemsMutex;
    pthread_cond_t itemsCond;
} XanPipe;

/**
 * @brief Make a new pipe.
 * \ingroup XanPipe
 *
 * Make a new pipe.
 * @param allocFunction This function is called each time the pipe needs to allocate new memory; if it is NULL, then the pipe will use the malloc function.
 * @param freeFunction This function is called each time the pipe needs to free memory; if it is NULL, then the pipe will use the free function.
 * @result Return the new pipe.
 */
XanPipe * xanPipe_new(void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr));

/**
 * @brief Get the oldest element from the pipe.
 * @ingroup XanPipe
 *
 * Get the oldest element from @c pipe.  If @c pipe is empty then wait until someone puts something into it.
 *
 * @param pipe The pipe to get an element from.
 * @return The oldest data element from @c pipe.
 */
void * xanPipe_get(XanPipe *pipe);

/**
 * @brief Put a data element into a pipe.
 * @ingroup XanPipe
 *
 * Put @c data into @c pipe, then signal to any waiting threads that there is data available.
 *
 * @param pipe The pipe to place @c data into.
 * @param data The data to insert into @c pipe.
 * @return A handle onto @c data as it is stored within @c pipe.
 */
XanListItem * xanPipe_put(XanPipe *pipe, void *data);

/**
 * @brief Put a data element into a pipe at the end.
 * @ingroup XanPipe
 *
 * Put @c data into the end of @c pipe (i.e. at the oldest position), then signal to any waiting threads that there is data available.
 *
 * @param pipe The pipe to place @c data into.
 * @param data The data to insert into @c pipe.
 * @return A handle onto @c data as it is stored within @c pipe.
 */
XanListItem * xanPipe_putAtEnd(XanPipe *pipe, void *data);

/**
 * @brief Move a data element to the end of a pipe.
 * @ingroup XanPipe
 *
 * Move @c data to the end of @c pipe (i.e. to the oldest position), then signal to any waiting threads that there is data available.
 *
 * @param pipe The pipe to move @c data within.
 * @param data The data that should be moved.
 * @return A handle onto the data as it is stored within @c pipe.
 */
XanListItem * xanPipe_moveToEnd(XanPipe *pipe, void *data);

/**
 * @brief Remove a data element from a pipe.
 * @ingroup XanPipe
 *
 * Remove the data that is linked to @c handle from @c pipe.  The data is unaffected.
 *
 * @param pipe The pipe to remove from.
 * @param handle The handle on the data that should be removed.
 */
void xanPipe_removeItem(XanPipe *pipe, XanListItem *handle);

/**
 * @brief Check whether a pipe is empty.
 * @ingroup XanPipe
 *
 * Check whether @c pipe contains any data.
 *
 * @param pipe The pipe to check.
 * @return @c true if the pipe is empty, @c false otherwise.
 */
bool xanPipe_isEmpty(XanPipe *pipe);

/**
 * @brief Destroy a pipe.
 * @ingroup XanPipe
 *
 * Destroy @c pipe by calling its free function.  This does not destroy any data stored in @c pipe.
 *
 * @param pipe The pipe to destroy.
 */
void xanPipe_destroyPipe(XanPipe *pipe);

/**
 * @brief Lock a pipe.
 * @ingroup XanPipe
 *
 * Lock the mutex associated with @c pipe.
 *
 * @param pipe The pipe to lock.
 */
void xanPipe_lock(XanPipe *pipe);

/**
 * @brief Unlock a pipe.
 * @ingroup XanPipe
 *
 * Unlock the mutex associated with @c pipe.
 *
 * @param pipe The pipe to unlock.
 */
void xanPipe_unlock(XanPipe *pipe);


/**
 * @brief Get the oldest element from the pipe, thread-safe.
 * @ingroup XanPipe
 *
 * Get the oldest element from @c pipe.  If @c pipe is empty then wait until someone puts something into it.
 *
 * @param pipe The pipe to get an element from.
 * @return The oldest data element from @c pipe.
 */
void * xanPipe_syncGet(XanPipe *pipe);

/**
 * @brief Put a data element into a pipe, thread-safe.
 * @ingroup XanPipe
 *
 * Put @c data into @c pipe, then signal to any waiting threads that there is data available.
 *
 * @param pipe The pipe to place @c data into.
 * @param data The data to insert into @c pipe.
 * @return A handle onto @c data as it is stored within @c pipe.
 */
XanListItem * xanPipe_syncPut(XanPipe *pipe, void *data);

/**
 * @brief Put a data element into a pipe at the end, thread-safe.
 * @ingroup XanPipe
 *
 * Put @c data into the end of @c pipe (i.e. at the oldest position), then signal to any waiting threads that there is data available.
 *
 * @param pipe The pipe to place @c data into.
 * @param data The data to insert into @c pipe.
 * @return A handle onto @c data as it is stored within @c pipe.
 */
XanListItem * xanPipe_syncPutAtEnd(XanPipe *pipe, void *data);

/**
 * @brief Move a data element to the end of a pipe, thread-safe.
 * @ingroup XanPipe
 *
 * Move @c data to the end of @c pipe (i.e. to the oldest position), then signal to any waiting threads that there is data available.
 *
 * @param pipe The pipe to move @c data within.
 * @param data The data that should be moved.
 * @return A handle onto the data as it is stored within @c pipe.
 */
XanListItem * xanPipe_syncMoveToEnd(XanPipe *pipe, void *data);

/**
 * @brief Remove a data element from a pipe, thread-safe.
 * @ingroup XanPipe
 *
 * Remove the data that is linked to @c handle from @c pipe.  The data is unaffected.
 *
 * @param pipe The pipe to remove from.
 * @param handle The handle on the data that should be removed.
 */
void xanPipe_syncRemoveItem(XanPipe *pipe, XanListItem *handle);

/**
 * @brief Check whether a pipe is empty, thread-safe.
 * @ingroup XanPipe
 *
 * Check whether @c pipe contains any data.
 *
 * @param pipe The pipe to check.
 * @return @c true if the pipe is empty, @c false otherwise.
 */
bool xanPipe_syncIsEmpty(XanPipe *pipe);


/**
 * #     #                  #####
 *  #   #     ##    #    # #     #  #    #  ######  #    #  ######
 *   # #     #  #   ##   # #     #  #    #  #       #    #  #
 *    #     #    #  # #  # #     #  #    #  #####   #    #  #####
 *   # #    ######  #  # # #   # #  #    #  #       #    #  #
 *  #   #   #    #  #   ## #    #   #    #  #       #    #  #
 * #     #  #    #  #    #  #### #   ####   ######   ####   ######
 **/

/**
 * @brief An item to hold data within a priority queue.
 * \ingroup XanQueue
 */
typedef struct XanQueueItem {
    float priority;
    int position;
    void    *element;
} XanQueueItem;

/**
 * @brief Priority Queue.
 * \ingroup XanQueue
 *
 * This is a priority queue structure, which can store unbounded elements inside itself.
 */
typedef struct XanQueue {
    void *          (*alloc)(size_t size);
    void (*free)(void *addr);
    void *          (*realloc)(void *addr, size_t size);

    XanQueueItem            **items;
    int maxSize;
    int size;
    pthread_mutex_t itemsMutex;
    pthread_cond_t itemsCond;
} XanQueue;

/**
 * @brief Make a priority queue.
 * \ingroup XanQueue
 *
 * Make a new priority queue.
 * @param allocFunction This function is called each time the queue needs to allocate new memory; if it is NULL, then the queue will use the malloc function.
 * @param reallocFunction This function is called each time the queue needs to reallocate memory; if it is NULL, then the queue will use the realloc function.
 * @param freeFunction This function is called each time the queue needs to free memory; if it is NULL, then the queue will use the free function.
 * @result Return the new queue.
 */
XanQueue * xanQueue_new(void *(*allocFunction)(size_t size), void *(*reallocFunction)(void *addr, size_t newSize), void (*freeFunction)(void *addr));

/**
 * @brief Get data from a priority queue.
 * @ingroup XanQueue
 *
 * Get the data element from @c queue with the highest priority.
 *
 * @param queue The queue to get a data element from.
 * @return The data element from @c queue with the highest priority.
 */
void * xanQueue_get(XanQueue *queue);

/**
 * @brief Put some data into a priority queue.
 * @ingroup XanQueue
 *
 * Put @c data into @c queue with a priority value of @c priority.
 *
 * @param queue The queue to place @c data into.
 * @param data The data to place into @c queue.
 * @param priority The priority value of @c data.
 * @return A handle onto @c data within @c queue.
 */
XanQueueItem *xanQueue_put(XanQueue *queue, void *data, float priority);

/**
 * @brief Find a data element within a priority queue.
 * @ingroup XanQueue
 *
 * Find @c data within @c queue and get a handle onto it.
 *
 * @param queue The queue to search in.
 * @param data The data to search for.
 * @return A handle onto @c data within @c queue, or @c NULL if @c data is not found.
 */
XanQueueItem *xanQueue_find(XanQueue *queue, void *data);

/**
 * @brief Check whether a priority queue is empty.
 * @ingroup XanQueue
 *
 * Check whether @c queue is empty.
 *
 * @param queue The queue to check.
 * @return @c true if the queue is empty, @c false otherwise.
 */
bool xanQueue_isEmpty(XanQueue *queue);

/**
 * @brief Get the priority value of the first data element within a priority queue.
 * @ingroup XanQueue
 *
 * Get the priority value of the data element in the head position within @c queue.
 *
 * @param queue The queue to look in.
 * @return The priority value of the head item in @c queue, or -1.0 if @c queue is empty.
 */
float xanQueue_headPriority(XanQueue *queue);

/**
 * @brief Change the priority value of an item in a priority queue.
 * @ingroup XanQueue
 *
 * Change the priority value of the data element stored in @c handle in @c queue to @c priority.
 *
 * @param queue The queue to consider.
 * @param handle A handle onto the data element whose priority to change.
 * @param priority The new priority value for the data element.
 */
void xanQueue_changeItemPriority(XanQueue *queue, XanQueueItem *handle, float priority);

/**
 * @brief Remove an item from a priority queue.
 * @ingroup XanQueue
 *
 * Remove the data element stored in @c handle from @c queue.  This does not affect the data element.
 *
 * @param queue The queue to remove from.
 * @param handle A handle onto the data element to remove.
 */
void xanQueue_removeItem(XanQueue *queue, XanQueueItem *handle);

/**
 * @brief Destroy a priority queue.
 * @ingroup XanQueue
 *
 * Destory @c queue by calling its free function.  This does not destroy any data stored in @c queue.
 *
 * @param queue The queue to destroy.
 */
void xanQueue_destroyQueue(XanQueue *queue);

/**
 * @brief Lock a priority queue.
 * @ingroup XanQueue
 *
 * Lock the mutex associated with @c queue.
 *
 * @param queue The queue to lock.
 */
void xanQueue_lock(XanQueue *queue);

/**
 * @brief Unlock a priority queue.
 * @ingroup XanQueue
 *
 * Unlock the mutex associated with @c queue.
 *
 * @param queue The queue to unlock.
 */
void xanQueue_unlock(XanQueue *queue);


/**
 * @brief Get data from a priority queue, thread-safe.
 * @ingroup XanQueue
 *
 * Get the data element from @c queue with the highest priority.
 *
 * @param queue The queue to get a data element from.
 * @return The data element from @c queue with the highest priority.
 */
void * xanQueue_syncGet(XanQueue *queue);

/**
 * @brief Put some data into a priority queue, thread-safe.
 * @ingroup XanQueue
 *
 * Put @c data into @c queue with a priority value of @c priority.
 *
 * @param queue The queue to place @c data into.
 * @param data The data to place into @c queue.
 * @param priority The priority value of @c data.
 * @return A handle onto @c data within @c queue.
 */
XanQueueItem *xanQueue_syncPut(XanQueue *queue, void *data, float priority);

/**
 * @brief Find a data element within a priority queue, thread-safe.
 * @ingroup XanQueue
 *
 * Find @c data within @c queue and get a handle onto it.
 *
 * @param queue The queue to search in.
 * @param data The data to search for.
 * @return A handle onto @c data within @c queue, or @c NULL if @c data is not found.
 */
XanQueueItem *xanQueue_syncFind(XanQueue *queue, void *data);

/**
 * @brief Check whether a priority queue is empty, thread-safe.
 * @ingroup XanQueue
 *
 * Check whether @c queue is empty.
 *
 * @param queue The queue to check.
 * @return @c true if the queue is empty, @c false otherwise.
 */
bool xanQueue_syncIsEmpty(XanQueue *queue);

/**
 * @brief Get the priority value of the first data element within a priority queue, thread-safe.
 * @ingroup XanQueue
 *
 * Get the priority value of the data element in the head position within @c queue.
 *
 * @param queue The queue to look in.
 * @return The priority value of the head item in @c queue, or -1.0 if @c queue is empty.
 */
float xanQueue_syncHeadPriority(XanQueue *queue);

/**
 * @brief Change the priority value of an item in a priority queue, thread-safe.
 * @ingroup XanQueue
 *
 * Change the priority value of the data element stored in @c handle in @c queue to @c priority.
 *
 * @param queue The queue to consider.
 * @param handle A handle onto the data element whose priority to change.
 * @param priority The new priority value for the data element.
 */
void xanQueue_syncChangeItemPriority(XanQueue *queue, XanQueueItem *handle, float priority);

/**
 * @brief Remove an item from a priority queue, thread-safe.
 * @ingroup XanQueue
 *
 * Remove the data element stored in @c handle from @c queue.  This does not affect the data element.
 *
 * @param queue The queue to remove from.
 * @param handle A handle onto the data element to remove.
 */
void xanQueue_syncRemoveItem(XanQueue *queue, XanQueueItem *handle);


/**
 * #     #                  #####
 *  #   #     ##    #    # #     #   #####    ##     ####   #    #
 *   # #     #  #   ##   # #           #     #  #   #    #  #   #
 *    #     #    #  # #  #  #####      #    #    #  #       ####
 *   # #    ######  #  # #       #     #    ######  #       #  #
 *  #   #   #    #  #   ## #     #     #    #    #  #    #  #   #
 * #     #  #    #  #    #  #####      #    #    #   ####   #    #
 **/

/**
 * @brief Stack
 * \ingroup XanStack
 *
 * This is a stack structure. This is a queue with the LIFO (Last In First Out) policy.
 */
typedef struct XanStack {
    XanList         *internalList;
    pthread_mutex_t mutex;
    void *                  (*alloc)(size_t size);                  /**< Alloc function				*/
    void 			(*free)	(void *address);                /**< Free function 				*/
    void *                  (*clone)(void *data);                   /**< Clone function. It clones the data field stored addressed by the elements inside the stack	*/
} XanStack;

/**
 * @brief Make a new stack.
 * \ingroup XanStack
 *
 * Create a new stack.
 *
 * @param allocFunction This function is called each time the stack needs to allocate new memory; if it is NULL, then the stack will use the malloc function.
 * @param freeFunction This function is called each time the stack needs to free memory; if it is NULL, then the stack will use the free function.
 * @param cloneFunction This function is called each time the stack needs to clone the variable stored inside its elements; if it is NULL, then the stack does not clone the variable stored inside each element of the stack.
 * @result Return the new stack.
 */
XanStack * xanStack_new(void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void * (*cloneFunction)(void *data));

/**
 * @brief Push a data element onto a stack.
 * \ingroup XanStack
 *
 * Push @c newElement onto @c stack.
 *
 * @param stack The stack to consider.
 * @param newElement The new data element to push.
 */
void xanStack_push (XanStack *stack, void *newElement);

/**
 * @brief Pop the top data element off a stack.
 * \ingroup XanStack
 *
 * Pop the top data element off @c stack and return it. If @c stack is empty then return @c NULL.
 *
 * @param stack The stack to consider.
 * @return The top data element from @c stack, or @c NULL if it was empty.
 */
void * xanStack_pop (XanStack *stack);

/**
 * @brief Get the data element on the top of a stack.
 * \ingroup XanStack
 *
 * Get the data element on top of @c stack, without altering @c stack at all.
 *
 * @param stack The stack to consider.
 * @return The data element stored on top of @c stack.
 */
void * xanStack_top (XanStack *stack);

/**
 * @brief Get the number of data elements stored on a stack.
 * \ingroup XanStack
 *
 * Get the number of data elements stored on @c stack.
 *
 * @param stack The stack to consider.
 * @return The number of data elements on @c stack.
 */
int xanStack_getSize (XanStack *stack);

/**
 * @brief Check whether a stack contains a data element.
 * @ingroup XanStack
 *
 * Check whether @c element is already on @c stack.
 *
 * @param stack The stack to check.
 * @param element The data element to look for.
 * @return true if @c element is on @c stack, false otherwise.
 */
bool xanStack_contains (XanStack *stack, void *element);

/**
 * @brief Check whether two stacks contain the same data elements.
 * @ingroup XanStack
 *
 * Check whether @c stack1 and @c stack2 contain exactly the same elements in any order, with no extras in either.
 *
 * @param stack1 The first stack to consider.
 * @param stack2 The second stack to consider.
 * @return true if both stacks contain exactly the same elements, false otherwise.
 */
bool xanStack_containsTheSameElements (XanStack *stack1, XanStack *stack2);

/**
 * @brief Create a list of data elements that are on a stack.
 * @ingroup XanStack
 *
 * Create a new list containing data elements that are on @c stack.
 *
 * @param stack The stack to consider.
 * @return A new list containing data elements on @c stack.
 */
XanList * xanStack_toList (XanStack *stack);

/**
 * @brief Clone a stack.
 * @ingroup XanStack
 *
 * Clone a stack and all of its data elements too, if its clone function is not @c NULL.
 *
 * @param stack The stack to clone.
 * @return The cloned stack.
 */
XanStack * xanStack_clone (XanStack *stack);

/**
 * @brief Destroy a stack.
 * \ingroup XanStack
 *
 * Free memory used by @c stack. Data elements stored on the stack are not freed.
 *
 * @param stack The stack to destroy.
 */
void xanStack_destroyStack (XanStack *stack);


/**
 * @brief Push a data element onto a stack, thread-safe.
 * \ingroup XanStack
 *
 * Push @c newElement onto @c stack.
 *
 * @param stack The stack to consider.
 * @param newElement The new data element to push.
 */
void xanStack_syncPush (XanStack *stack, void *newElement);

/**
 * @brief Pop the top data element off a stack, thread-safe.
 * \ingroup XanStack
 *
 * Pop the top data element off @c stack and return it. If @c stack is empty then return @c NULL.
 *
 * @param stack The stack to consider.
 * @return The top data element from @c stack, or @c NULL if it was empty.
 */
void * xanStack_syncPop (XanStack *stack);

/**
 * @brief Get the data element on the top of a stack, thread-safe.
 * \ingroup XanStack
 *
 * Get the data element on top of @c stack, without altering @c stack at all.
 *
 * @param stack The stack to consider.
 * @return The data element stored on top of @c stack.
 */
void * xanStack_syncTop (XanStack *stack);

/**
 * @brief Get the number of data elements stored on a stack, thread-safe.
 * \ingroup XanStack
 *
 * Get the number of data elements stored on @c stack.
 *
 * @param stack The stack to consider.
 * @return The number of data elements on @c stack.
 */
int xanStack_syncGetSize (XanStack *stack);

/**
 * @brief Check whether a stack contains a data element, thread-safe.
 * @ingroup XanStack
 *
 * Check whether @c element is already on @c stack.
 *
 * @param stack The stack to check.
 * @param element The data element to look for.
 * @return true if @c element is on @c stack, false otherwise.
 */
bool xanStack_syncContains (XanStack *stack, void *element);

/**
 * \ingroup XanHashTableAllocate
 * @brief Make a new Hash Table.
 *
 * Allocate and initialize a new Hash table. After the allocation, the hash and equals functions cannot be changed.
 *
 * @param length The initial size of the hash table. It will grow in size automatically as elements are added.
 * @param hasFixedLength Whether the size of the hash table can grow automatically or not.
 * @param allocFunction This function is called each time the hash table needs to allocate new memory; if it is NULL, then the hash table will use the malloc function.
 * @param reallocFunction This function is called each time the hash table needs to reallocate memory; if it is NULL, then the hash table will use the realloc function.
 * @param freeFunction This function is called each time the hash table needs to free memory; if it is NULL, then the hash table will use the free function.
 * @param hashFunction This function is called to compute a hash on the key elements; if it is NULL, then the hash table will use a simple default hash function.
 * @param equalsFunction This function is called to determine whether two keys are equal; if it is NULL, then the hash table will use a simple default comparison function.
 * @result Return the new hash table.
 */
XanHashTable * xanHashTable_new(unsigned int length, int hasFixedLength, void *(*allocFunction)(size_t size), void *(*reallocFunction)(void *addr, size_t newSize), void (*freeFunction)(void *addr), unsigned int (*hashFunction)(void *element), int (*equalsFunction)(void *key1, void *key2));

/**
 * \ingroup XanHashTableAdd
 * @brief Insert a new element
 *
 * Add the pointer @c element into the hash table @c table by using @c key as its identifier.
 *
 * @param table Table to consider
 * @param key Identificator of @c element
 * @param element Element to insert in the table
 */
void xanHashTable_insert (XanHashTable *table, void *key, void *element);

/**
 * \ingroup XanHashTable
 * @brief Replace an existing element with a new one
 *
 * Replace the element that is pointed to by @c key in the hash table @c table to point to pointer @c element.
 * If @c key is not found in the hash table, do nothing.
 *
 * @param table Table to consider
 * @param key Identificator of @c element
 * @param element Element to replace with in the table
 */
void xanHashTable_replace (XanHashTable *table, void *key, void *element);

/**
 * \ingroup XanHashTable
 * @brief Lookup an element
 *
 * Lookup from the hash table @c table the element that has @c key as its identifier.
 *
 * Return NULL if no such element exist.
 *
 * @param table Table to consider
 * @param key Identificator the returned element
 * @return Element with identifier @c key that is stored within @c table
 */
void * xanHashTable_lookup (XanHashTable *table, void *key);

/**
 * \ingroup XanHashTable
 * @brief Lookup an element
 *
 * Lookup from the hash table @c table the element that has @c key as its identifier.
 *
 * The returned address is the pointer to the internal structure used to store the element into the table @c table.
 *
 * Return NULL if no such element exist.
 *
 * @param table Table to consider
 * @param key Identificator the returned element
 * @return Pointer to the data structure that contains the element with identifier @c key that is stored within @c table
 */
XanHashTableItem * xanHashTable_lookupItem (XanHashTable *table, void *key);

/**
 * \ingroup XanHashTableAdd
 * @brief Add a set of elements to a table
 *
 * Add every element stored in @c listToAdd  to @c table .
 *
 * Each element is used for both the key and the value of the table.
 *
 * @param table Table to consider
 * @param listToAdd Elements to add
 */
void xanHashTable_addList (XanHashTable *table, XanList *listToAdd);

/**
 * \ingroup XanHashTable
 * @brief Get the first element
 *
 * Fetch the first element inserted in to the table @c table.
 *
 * Return NULL if the table is empty.
 *
 * @param table Table to consider
 * @return First element stored into the table
 */
XanHashTableItem * xanHashTable_first (XanHashTable* table);

/**
 * \ingroup XanHashTable
 * @brief Get the next element
 *
 * Fetch the next element inserted in to the table @c table.
 *
 * Return NULL if @c item is the last element stored into the table.
 *
 * @param table Table to consider
 * @param item Element stored just before the returned one
 * @return Next element stored into the table
 */
XanHashTableItem * xanHashTable_next (XanHashTable* table, XanHashTableItem* item);

/**
 * \ingroup XanHashTable
 * @brief Number of elements
 *
 * Return the number of elements stored in @c table.
 *
 * @param table Table to consider
 * @return Number of elements stored in @c table
 */
int xanHashTable_elementsInside (XanHashTable *table);

/**
 * \ingroup XanHashTableRemove
 * @brief Removed an element
 *
 * Find and remove the element with key @c key .
 *
 * The returned element is the one has been removed from the table.
 *
 * @param table Hash table to consider
 * @param key Key of the removed element
 * @return Removed element
 */
void * xanHashTable_removeElement (XanHashTable *table, void *key);

/**
 * \ingroup XanHashTableRemove
 * @brief Removed a set of elements
 *
 * Find and remove the elements with a key listed in @c keysToRemove .
 *
 * @param table Hash table to consider
 * @param keysToRemove Keys to consider to remove the elements
 */
void xanHashTable_removeElements (XanHashTable *table, XanList *keysToRemove);

/**
 * \ingroup XanHashTableRemove
 * @brief Removed an element, key pair
 *
 * Find and remove the pair @c key, @c data .
 *
 * @param table Hash table to consider
 * @param key Key of the removed element
 * @param data Data of the removed element
 */
void xanHashTable_removeElementKeyPair (XanHashTable *table, void *key, void *data);

/**
 * \ingroup XanHashTableRemove
 * @brief Removed an item.
 *
 * Removed an internal item from the table.
 *
 * @param table Hash table to consider
 * @param item Item to remove
 */
void xanHashTable_removeItem (XanHashTable *table, XanHashTableItem *item);

/**
 * \ingroup XanHashTableRemove
 * @brief Flush the elements in a table
 *
 * Remove every element stored in @c table .
 *
 * @param table Table to consider
 */
void xanHashTable_emptyOutTable (XanHashTable *table);

/**
 * \ingroup XanHashTableRemove
 * @brief Flush the elements in a table
 *
 * Remove every element stored in @c table and call the free function stored inside @c table to destroy the keys.
 *
 * @param table Table to consider
 */
void xanHashTable_emptyOutTableFreeKey (XanHashTable *table);

/**
 * \ingroup XanHashTableRemove
 * @brief Flush the elements in a table
 *
 * Remove every element stored in @c table and call the free function stored inside @c table to destroy the keys and data.
 *
 * @param table Table to consider
 */
void xanHashTable_emptyOutTableFreeDataAndKey (XanHashTable *table);

/**
 * \ingroup XanHashTableRemove
 * @brief Flush and free the elements in a table
 *
 * Remove every element stored in @c table and call the free function to them.
 *
 * @param table Table to consider
 */
void xanHashTable_deleteAndFreeElements (XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 * @brief Convert to list
 *
 * Convert the table to a list.
 *
 * Only elements added in the table will be included in the returned list.
 *
 * @param table Table to consider
 * @return List of elements stored in @c table
 */
XanList * xanHashTable_toList(XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 * @brief Get a list of keys into the hash table.
 *
 * Returns the list of elementIDs that are part of the hashtable.
 */
XanList * xanHashTable_toKeyList (XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 * @brief Returns the list of data structures used to store elements in a table
 *
 * Returns the list of data structures used to store elements in @c table.
 *
 * @param table Table to consider
 */
XanList * xanHashTable_toSlotList (XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 * @brief From HashTable to List
 *
 * Returns the list of element and elementID pairs (in the form of XanHashTableItem) that are part of the hashtable.
 *
 * These are only copies of the XanHashTableItems of the hashtable, and modifing them does not affect the original hashtable.
 *
 * Each of the Items has to be deallocated while deallocating the XanList (e.g. calling the function destroyListAndData of the XanList data structure).
 *
 */
XanList * xanHashTable_toItemList (XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 * @brief Get an array of keys into the hash table.
 *
 * Returns an array of elementIDs that are part of the hashtable.  The number of elements can be found by calling @c xanHashTable_elementsInside() on @c table.
 */
void ** xanHashTable_toKeyArray (XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 * @brief Get an array of elements from the hash table.
 *
 * Returns an array of elements that are part of the hashtable.  The number of elements can be found by calling @c xanHashTable_elementsInside() on @c table.
 */
void ** xanHashTable_toDataArray (XanHashTable *table);

/**
 * \ingroup XanHashTableAllocate
 * @brief Clone a table
 *
 * Clone the table given as input (i.e., @c table).
 *
 * The function "clone" stored inside @c table is used to clone each element.
 *
 * @return Cloned table
 */
XanHashTable * xanHashTable_cloneTable(XanHashTable *table);

/**
 * \ingroup XanHashTableDestroy
 * @brief Destroy the table
 *
 * Free the memory used by the hash table.
 *
 * @param table Table to consider
 */
void xanHashTable_destroyTable (XanHashTable *table);

/**
 * \ingroup XanHashTableDestroy
 * @brief Destroy the table
 *
 * Free the memory used by the hash table and call the function free stored inside @c table to destroy the elements stored inside @c table.
 *
 * This function does not destroy the identifiers of the elements.
 *
 * @param table Table to consider
 */
void xanHashTable_destroyTableAndData(XanHashTable *table);

/**
 * \ingroup XanHashTableDestroy
 * @brief Destroy the table
 *
 * Free the memory used by the hash table and call the function free stored inside @c table to destroy the identifiers of the elements stored inside @c table.
 *
 * This function does not destroy the elements in @c table.
 *
 * @param table Table to consider
 */
void xanHashTable_destroyTableAndKey(XanHashTable *table);

/**
 * \ingroup XanHashTableDestroy
 * @brief Destroy the table
 *
 * Free the memory used by the hash table and call the function free stored inside @c table to destroy both the elements and their identifiers stored inside @c table.
 *
 * @param table Table to consider
 */
void xanHashTable_destroyTableDataAndKey(XanHashTable *table);

/**
 * \ingroup XanHashTable
 * @brief Return the clone function.
 *
 * Return the function used to clone elements stored in @c table.
 *
 * @param table Table to consider
 * @return Clone function
 */
void * xanHashTable_getCloneFunction (XanHashTable *table);

/**
 * \ingroup XanHashTable
 * @brief Set the clone function.
 *
 * Set the function callable by the hash table when it has to clone itself and then it has to clone the inside elements also. If the clone function is set to be NULL, then when the hash table clone itself, it does not clone the internal elements (it makes the shallow copy).
 */
void xanHashTable_setCloneFunction (XanHashTable *table, void * (*cloneFunction)(void *data));

/**
 * \ingroup XanHashTable
 *
 * @param table Table to consider
 */
void xanHashTable_lock (XanHashTable *table);

/**
 * \ingroup XanHashTable
 *
 * @param table Table to consider
 */
void xanHashTable_unlock (XanHashTable *table);

/**
 * \ingroup XanHashTableAdd
 *
 * @param table Table to consider
 */
void xanHashTable_syncInsert (XanHashTable *table, void *key, void *element);

/**
 * \ingroup XanHashTable
 *
 * @param table Table to consider
 */
void * xanHashTable_syncLookup (XanHashTable *table, void *key);

/**
 * \ingroup XanHashTable
 *
 * @param table Table to consider
 */
XanHashTableItem * xanHashTable_syncFirst (XanHashTable* table);

/**
 * \ingroup XanHashTable
 *
 * @param table Table to consider
 */
XanHashTableItem * xanHashTable_syncNext (XanHashTable* table, XanHashTableItem* item);

/**
 * \ingroup XanHashTable
 *
 * Return the number of elements stored inside the hash table. This operation is act only after taked the lock of the hash table.
 */
int xanHashTable_syncElementsInside (XanHashTable *table);

/**
 * \ingroup XanHashTableRemove
 *
 * @param table Table to consider
 */
void * xanHashTable_syncRemoveElement (XanHashTable *table, void *key);

/**
 * \ingroup XanHashTableRemove
 *
 * @param table Table to consider
 */
void xanHashTable_syncEmptyOutTable (XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 *
 * @param table Table to consider
 */
XanList * xanHashTable_syncToList(XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 *
 * @param table Table to consider
 */
XanList * XanHashTable_syncToKeyList (XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 *
 * @param table Table to consider
 */
XanList * xanHashTable_syncToSlotList(XanHashTable *table);

/**
 * \ingroup XanHashTableConversion
 *
 * @param table Table to consider
 */
XanList * XanHashTable_syncToItemList (XanHashTable *table);

/**
 * \ingroup XanHashTableAllocate
 *
 * @param table Table to consider
 */
XanHashTable * xanHashTable_syncCloneTable(XanHashTable *table);

/**
 * @brief Return the clone function.
 * \ingroup XanHashTable
 *
 * Return the clone function.
 *
 * This operation is act only after taked the lock of the hash table.
 *
 * @param table Table to consider
 */
void * xanHashTable_syncGetCloneFunction (XanHashTable *table);

/**
 * @brief Set the clone function.
 * \ingroup XanHashTable
 *
 * Set the function callable by the hash table when it has to clone itself and then it has to clone the inside elements also.
 *
 * If the clone function is set to be NULL, then when the hash table clone itself, it does not clone the internal elements (it makes the shallow copy).
 *
 * This operation is act only after taked the lock of the hash table.
 *
 * @param table Table to consider
 */
void xanHashTable_syncSetCloneFunction (XanHashTable *table, void * (*cloneFunction)(void *data));

/**
 * @brief Node of a generic tree.
 * \ingroup XanNode
 *
 * The XanNode structure implements a generic N-ary tree.
 */
typedef struct XanNode {

    /* Fields				*/
    XanList         *childrens;
    void            *data;
    struct XanNode  *parent;
    pthread_mutex_t mutex;
    void *                  (*alloc)(size_t size);
    void (*free)(void *address);
    void *                  (*clone)(void *data);

    /* Methods synchronized			*/
    struct XanNode *        (*synchGetParent)(struct XanNode *node);                                /**< Return the parent of the node. If this node has no parent, then returns a NULL pointer. This operation is act only after taked the lock to the tree.       */
    struct XanNode *        (*synchGetNextChildren)(struct XanNode *node, struct XanNode *child);   /**< Return the next children of the one given as input. If the child parameter is NULL, then it returns the first child of the node. This operation is act only after taked the lock to the tree.	*/
    struct XanNode *        (*synchAddNewChildren)(struct XanNode *node, void *childData);          /**< Make a new XanNode struct which stores inside itself the childData parameter, before returning it, it adds this new node to the list of childrens of the node. This operation is act only after taked the lock to the tree.	*/
    struct XanNode *        (*synchCloneTree)(struct XanNode *node);                                /**< Return a clone of the tree; each element stored inside each node of the tree rooted to the current one is cloned by calling the cloning function of the node. This operation is act only after taked the lock to the tree.	*/
    void (*synchAddChildren)(struct XanNode *node, struct XanNode *child);                          /**< Add the node given as input to the childrens of the current one. This operation is act only after taked the lock to the tree.	*/
    void (*synchDeleteChildren)(struct XanNode *node, struct XanNode *child);                       /**< Delete the children given as input from the list of childrens of the current node. This operation is act only after taked the lock to the tree.*/
    void (*synchSetParent)(struct XanNode *node, struct XanNode *parent);                           /**< Set the node given as input as the parent of the current one. This operation is act only after taked the lock to the tree.	*/
    void (*synchSetData)(struct XanNode *node, void *data);                                         /**< Set the data stored inside the current node. This operation is act only after taked the lock to the tree.	*/
    void *                  (*synchGetData)(struct XanNode *node);                                  /**< Return the data stored inside the current node. This operation is act only after taked the lock to the tree.	*/
    XanList *               (*synchGetChildrens)(struct XanNode *node);                             /**< Return the list of childrens of the current node. This operation is act only after taked the lock to the tree.	*/
    void (*synchSetCloneFunction)(struct XanNode *node, void * (*cloneFunction)(void *data));       /**< Set the clone function of the current node. This operation is act only after taked the lock to the tree.	*/
    void (*synchDestroyTree)(struct XanNode *node);                                                 /**< Destroy the tree rooted to the current node	 */
    void (*synchDestroyTreeAndData)(struct XanNode *node);                                          /**< Destroy the tree rooted to the current node, freeing the data stored inside each element of this tree calling the free function of the current node. This operation is act only after taked the lock to the tree.	*/
    XanList *               (*synchToPreOrderList)(struct XanNode *rootNode);                       /**< Convert the tree rooted at rootNode to the pre-order list. This operation is act only after taked the lock to the tree.	*/
    XanList *               (*synchToPostOrderList)(struct XanNode *rootNode);                      /**< Convert the tree rooted at rootNode to the post-order list. This operation is act only after taked the lock to the tree.	*/
    XanList *               (*synchToInOrderList)(struct XanNode *rootNode);                        /**< Convert the tree rooted at rootNode to the in-order list. This operation is act only after taked the lock to the tree.	*/
    struct XanNode *        (*synchFind)(struct XanNode *rootNode, void *data);                     /**< Find the value `data` inside the tree rooted from rootNode. This operation is act only after taked the lock to the tree. */

    /* Methods not synchronized		*/
    struct XanNode *        (*getParent)(struct XanNode *node);                                     /**< Return the parent of the node. If this node has no parent, then returns a NULL pointer.    */
    struct XanNode *        (*getNextChildren)(struct XanNode *node, struct XanNode *child);        /**< Return the next children of the one given as input. If the child parameter is NULL, then it returns the first child of the node. */
    struct XanNode *        (*addNewChildren)(struct XanNode *node, void *childData);               /**< Make a new XanNode struct which stores inside itself the childData parameter, before returning it, it adds this new node to the list of childrens of the node.     */
    struct XanNode *        (*cloneTree)(struct XanNode *node);                                     /**< Return a clone of the tree; each element stored inside each node of the tree rooted to the current one is cloned by calling the cloning function of the node.      */
    void (*addChildren)(struct XanNode *node, struct XanNode *child);                               /**< Add the node given as input to the childrens of the current one.   */
    void (*deleteChildren)(struct XanNode *node, struct XanNode *child);                            /**< Delete the children given as input from the list of childrens of the current node. */
    void (*setParent)(struct XanNode *node, struct XanNode *parent);                                /**< Set the node given as input as the parent of the current one.      */
    void (*setData)(struct XanNode *node, void *data);                                              /**< Set the data stored inside the current node.       */
    void *                  (*getData)(struct XanNode *node);                                       /**< Return the data stored inside the current node. */
    XanList *               (*getChildrens)(struct XanNode *node);                                  /**< Return the list of childrens of the current node.	*/
    void (*setCloneFunction)(struct XanNode *node, void * (*cloneFunction)(void *data));            /**< Set the clone function of the current node. */
    void (*destroyTree)(struct XanNode *node);                                                      /**< Destroy the tree rooted to the current node	 */
    void (*destroyNode)(struct XanNode *node);                                                      /**< Destroy the node given as input			 */
    void (*destroyTreeAndData)(struct XanNode *node);                                               /**< Destroy the tree rooted to the current node, freeing the data stored inside each element of this tree calling the free function of the current node. */
    XanList *               (*toPreOrderList)(struct XanNode *rootNode);                            /**< Convert the tree rooted at rootNode to the pre-order list. Each element of the list has the same type as `data`.	*/
    XanList *               (*toPostOrderList)(struct XanNode *rootNode);                           /**< Convert the tree rooted at rootNode to the post-order list. Each element of the list has the same type as `data`.	*/
    XanList *               (*toInOrderList)(struct XanNode *rootNode);                             /**< Convert the tree rooted at rootNode to the in-order list. Each element of the list has the same type as `data`.	*/
    struct XanNode *        (*find)(struct XanNode *rootNode, void *data);                          /**< Find the value `data` inside the tree rooted from rootNode. */
    unsigned int (*getDepth)(struct XanNode *rootNode); /***< Get the total depth of the tree rooted at rootNode. */
} XanNode;

/**
 * @brief Graph node
 * \ingroup XanGraph
 *
 * This structure describes a node in a graph
 */
typedef struct {
    XanHashTable 	*incomingEdges;	/**< List of incoming edges of the node. Each element is of type (XanGraphEdge *).		*/
    XanHashTable	*outgoingEdges;	/**< List of outgoing edges of the node. Each element is of type (XanGraphEdge *).		*/
    void		*data;		/**< Element stored inside the node								*/
} XanGraphNode ;

/**
 * @brief Graph edge
 * \ingroup XanGraph
 *
 * This structure describes an edge in a graph
 */
typedef struct {
    void		*data;	/**< Element stored inside the edge							*/
    XanGraphNode	*p1;	/**< Source or destination of the edge							*/
    XanGraphNode	*p2;	/**< Source or destination of the edge							*/
} XanGraphEdge ;

/**
 * @brief Graph
 * \ingroup XanGraph
 *
 * This structure describes a graph of nodes
 */
typedef struct {
    void *    	(*alloc)		(size_t size);
    void 		(*free)			(void *addr);
    void *		(*realloc)		(void *addr, size_t newSize);

    void		*data;		/**< Element stored inside the graph	*/
    XanHashTable	*nodes;		/**< Nodes of the graph. Elements are (XanGraphNode *). Keys are the data given as input when nodes are created.			*/
    XanHashTable	*edges;		/**< Edges of the graph. Elements are (XanGraphEdge *). Keys are the data given as input when nodes are created.			*/
} XanGraph ;

/**
 * @brief Make a new graph
 * \ingroup XanGraph
 *
 * Allocate and return a new graph
 *
 * @param allocFunction This function is called each time the graph needs to allocate new memory; if it is NULL, then the list will use the malloc function.
 * @param freeFunction This function is called each time the graph needs to free memory; if it is NULL, then the list will use the free function.
 * @result A new graph
 */
XanGraph * xanGraph_new (void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void *(*reallocFunction)(void *addr, size_t newSize), void *data);

/**
 * @brief Get an edge within a graph that contains specific data.
 * \ingroup XanGraph
 *
 * Return the edge in \c g that contains \c data.
 *
 * @param g The graph to consider
 * @param data The data that should exist within the returned edge
 * @return The edge in \c g containing \c data, if it exists in an edge, NULL otherwise
 */
XanGraphEdge * xanGraph_getEdge (XanGraph *g, void *data);

/**
 * @brief Get a directed edge in a graph between two nodes.
 * \ingroup XanGraph
 *
 * Return the edge in \c that connects \c n1 to \c n2.
 *
 * @param n1 The node the edge starts at
 * @param n2 The node the edge ends at
 * @return The edge connecting \c n1 and \c n2 if it exists, NULL otherwise
 */
XanGraphEdge * xanGraph_getDirectedEdge (XanGraphNode *n1, XanGraphNode *n2);

/**
 * @brief Get a node in a graph that contains specific data.
 * \ingroup XanGraph
 *
 * Return the node in \c g that contains \c data.
 *
 * @param g The graph to consider
 * @param data The data that should exist within the returned node
 * @return The node in \c g containing \c data, if it exists, NULL otherwise
 */
XanGraphNode * xanGraph_getNode (XanGraph *g, void *data);

/**
 * @brief Add a new node into a graph that contains specific data.
 * \ingroup XanGraph
 *
 * Create a new node in \c g that contains \c data and return that node.
 *
 * @param g The graph to consider
 * @param data The data to place in the new node
 * @return The new node that is created in \c g
 */
XanGraphNode * xanGraph_addANewNode (XanGraph *g, void *data);

/**
 * @brief Add a new node into a graph that contains specific data if it does not exist already
 * \ingroup XanGraph
 *
 * Create a new node in \c g that contains \c data and return that node if it does not exist already.
 *
 * Instead, if it already exists, this function returns it.
 *
 * @param g The graph to consider
 * @param data The data to place in the new node
 * @return The new node that is created in \c g
 */
XanGraphNode * xanGraph_addANewNodeIfNotExist (XanGraph *g, void *data);

/**
 * @brief Destroy a graph
 * \ingroup XanGraph
 *
 * Free the memory used by the graph <code> g </code>.
 *
 * @param g Graph to destroy
 */
void xanGraph_destroyGraph (XanGraph *g);

/**
 * @brief Add a directed edge
 * \ingroup XanGraph
 *
 * Add the edge from <code> n1 </code> to <code> n2 </code>.
 *
 */
XanGraphEdge * xanGraph_addDirectedEdge (XanGraph *g, XanGraphNode *n1, XanGraphNode *n2, void *data);

/**
 * @brief Add an undirected edge
 * \ingroup XanGraph
 *
 * Add the edge between <code> n1 </code> and <code> n2 </code>.
 */
void xanGraph_addUndirectedEdge (XanGraph *g, XanGraphNode *n1, XanGraphNode *n2, void *data);

/**
 * @brief Check if a directed edge exist inside the graph
 * \ingroup XanGraph
 *
 * Return 1 if there is a directed edge from <code> n1 </code> to <code> n2 </code>.
 */
int xanGraph_existDirectedEdge (XanGraphNode *n1, XanGraphNode *n2);

/* Tree				*/
/**
 * @brief Make a new node.
 * \ingroup XanNode
 *
 * Make a new node of the N-ary tree.
 * @param allocFunction This function is called each time the tree needs to allocate new memory; if it is NULL, then the tree will use the malloc function.
 * @param freeFunction This function is called each time the tree needs to free memory; if it is NULL, then the tree will use the free function.
 * @param cloneFunction This function is called each time the tree needs to clone the variable stored inside its elements; if it is NULL, then the tree does not clone the variable stored inside each element of the tree rooted at the current node.
 * @result Return the new node.
 */
XanNode * xanNode_new(void *(*allocFunction)(size_t size), void (*freeFunction)(void *addr), void * (*cloneFunction)(void *data));

/* Generic utilities		*/
/**
 * @brief Print an error message
 * \ingroup XanLib
 *
 * Print to stderr the error message given as input. If the err parameter is different from zero, then it append to the message the error message given bound to the err number.
 * @param message The error message to print
 * @param err The errno number given by the Linux system
 */
void print_err (char * message, int err);

/**
 * @brief Print an error message
 * \ingroup XanLib
 *
 * Print to stderr the error message given as input. If the err parameter is different from zero, then it append to the message the error message given bound to the err number.
 * @param message The error message to print
 * @param err The errno number given by the Linux system
 */
void print_ascii_err (signed char * message, int err);

/**
 * @brief Check if string begin with suffix.
 * \ingroup XanLib
 *
 * Check if the string begin with a suffix.
 * @param string Pointer to the string to check
 * @param suffix Pointer to string rappresent the suffix
 * @result 0 if the string pointed by the @var{string} parameter begin with the suffix pointed by the @var{suffix} parameter
 */
int str_has_suffix (char *string, char *suffix);

/**********************************************************************************************************
*                 xan_bitset
**********************************************************************************************************/

/**
 * @brief Bitset
 * \ingroup XanBitSet
 *
 * This structure describes a bitset.
 */
typedef struct XanBitSet {
    size_t *data; /**< Holds the bitset itself. */
    size_t length; /**< The number of bits in the bitset. */
    size_t capacity; /**< The number of size_t blocks allocated for the bitset. */
} XanBitSet;

/**
 * @brief Allocate a bitset structure.
 * \ingroup XanBitSet
 *
 * Allocates memory for a bitset and returns a pointer to it.
 *
 * @param length The length of the bitset.
 * @return A pointer to the new bitset.
 */
XanBitSet *xanBitSet_new (size_t length);

/**
 * @brief Set all bits to 0.
 * \ingroup XanBitSet
 *
 * Sets all bits in the bitset \c set to 0.
 *
 * @param set The bitset to clear.
 */
void xanBitSet_clearAll (XanBitSet *set);

/**
 * @brief Set all bits to 1.
 * \ingroup XanBitSet
 *
 * Set all bits in the bitset \c set to 1.
 *
 * @param set The bitset to set all bits in.
 */
void xanBitSet_setAll (XanBitSet *set);

/**
 * @brief Invert all bits in a bitset
 * \ingroup XanBitSet
 *
 * Inverts all bits in the bitset \c set so that bits that were 0 become 1 and bits that were 1 become 0.
 *
 * @param set The bitset to invert bits in.
 */
void xanBitSet_invertBits (XanBitSet *set);

/**
 * @brief Subtract one bitset from another.
 * \ingroup XanBitSet
 *
 * Subtract bitset \c subtractThis from bitset \c fromThis. The result ends up in \c fromThis, which is modified.
 *
 * @param fromThis The bitset to subtract \c subtractThis from.
 * @param subtractThis The bitset to subtract from \c fromThis.
 */
void xanBitSet_subtract (XanBitSet *fromThis, XanBitSet *subtractThis);

/**
 * @brief Free memory used by a bitset.
 * \ingroup XanBitSet
 *
 * Free memory used by the bitset \c set.
 *
 * @param set The bitset to free.
 */
void xanBitSet_free (XanBitSet *set);

/**
 * @brief Compute the intersection between two bitsets.
 * \ingroup XanBitSet
 *
 * Compute the intersection between \c dest and \c src and assign it to \c dest.
 *
 * @param dest One source bitset for the intersection and the destination bitset.
 * @param src The second source bitset for the intersection.
 */
void xanBitSet_intersect (XanBitSet *dest, XanBitSet *src);

/**
 * @brief Compute the union between two bitsets.
 * \ingroup XanBitSet
 *
 * Compute the union between \c dest and \c src and assign it to \c dest.
 *
 * @param dest One source bitset for the union and the destination bitset.
 * @param src The second source bitset for the union.
 */
void xanBitSet_union (XanBitSet *dest, XanBitSet *src);

/**
 * @brief Compute the number of bits set within a bitset.
 * \ingroup XanBitSet
 *
 * Count the number of bits that are 1 within \c set.
 *
 * @param set The bitset to consider.
 * @return The count of bits set in \c set.
 */
int xanBitSet_getCountOfBitsSet (XanBitSet *set);

/**
 * @brief Determine whether two bitsets are equal.
 * \ingroup XanBitSet
 *
 * Determine whether \c bs1 and \c bs2 have the same length and the same bits set to 1 and 0.
 *
 * @param bs1 The first bitset for comparison.
 * @param bs2 The second bitset for comparison.
 * @return true if \c bs1 and \c bs2 are equal, false otherwise.
 */
bool xanBitSet_equal (XanBitSet *bs1, XanBitSet *bs2);

/**
 * @brief Clone a bitset.
 * \ingroup XanBitSet
 *
 * Clone \c src and return the newly-created bitset.
 *
 * @param src The bitset to clone.
 * @return The newly-created bitset.
 */
XanBitSet* xanBitSet_clone (XanBitSet *src);

/**
 * @brief Copy a bitset into another.
 * \ingroup XanBitSet
 *
 * Copy \c src into \dest so that they are equal.
 *
 * @param dest The bitset to copy into.
 * @param src The bitset to copy from.
 */
void xanBitSet_copy (XanBitSet *dest, XanBitSet *src);

/**
 * @brief Clear a specific bit within a bitset.
 * \ingroup XanBitSet
 *
 * Clear the bit at position @c pos within @c set (i.e., set it to 0).
 *
 * @param set The bitset to clear a bit in.
 * @param pos The position of the bit to clear.
 */
void xanBitSet_clearBit (XanBitSet *set, size_t pos);

/**
 * @brief Set a specific bit within a bitset.
 * \ingroup XanBitSet
 *
 * Set the bit at position @c pos within @c set (i.e., set it to 1).
 *
 * @param set The bitset to set a bit in.
 * @param pos The position of the bit to set.
 */
void xanBitSet_setBit (XanBitSet *set, size_t pos);

/**
 * @brief Set specific bits within a bitset.
 * \ingroup XanBitSet
 *
 * Set the bits from position @c start to @c end inclusive within @c set.
 *
 * @param set The bitset to set bits in.
 * @param start The position of the first bit to set.
 * @param end The position of the last bit to set.
 */
void xanBitSet_setBits (XanBitSet *set, size_t start, size_t end);

/**
 * @brief Check whether a bit is set.
 * \ingroup XanBitSet
 *
 * Check whether the bit at position @c pos within @c set is set (i.e., whether it is 1).
 *
 * @param set The bitset to check within.
 * @param pos The position of the bit to check.
 * @return true if the bit at position @c pos is set, false otherwise.
 */
bool xanBitSet_isBitSet (XanBitSet *set, size_t pos);

/**
 * @brief Check whether one bitset is a subset of another.
 * \ingroup XanBitSet
 *
 * Check whether @c setA is a subset of @c setB, including them being equal.  Both sets must be the same length, otherwise false is returned.
 *
 * @param setA The bitset to check for being a subset of @c setB.
 * @param setB The bitset to check for being a superset of @c setA.
 * @return true if @c setA is a subset of @c setB, false otherwise.
 */
bool xanBitSet_isSubSetOf (XanBitSet *setA, XanBitSet *setB);

/**
 * @brief Check whether the intersection of two bitsets is empty.
 * \ingroup XanBitSet
 *
 * Check whether the intersection of @c setA and @c setB is empty.
 *
 * @param setA The first bitset to consider.
 * @param setB The second bitset to consider.
 * @return true if the intersection of @c setA and @c setB is empty, false otherwise.
 */
bool xanBitSet_isIntersectionEmpty (XanBitSet *setA, XanBitSet *setB);

/**
 * @brief Check whether a bitset is empty.
 * \ingroup XanBitSet
 *
 * Check whether @c set is empty.  I.e. it has no bits set.
 *
 * @param set The set to check for being empty.
 * @return true if @c set is empty, false otherwise.
 */
bool xanBitSet_isEmpty (XanBitSet *set);

/**
 * @brief Check whether two bitsets are equal.
 * @ingroup XanBitSet
 *
 * Check whether @c setA has exactly the same bits set as @c setB.
 *
 * @param setA The first bitset to consider.
 * @param setB The second bitset to consider.
 * @return true if the two bitsets are equal, false otherwise.
 */
bool xanBitSet_areEqual (XanBitSet *setA, XanBitSet *setB);

/**
 * @brief Print a bitset to stdout.
 * \ingroup XanBitSet
 *
 * Print @c set to stdout. Include a new line if @cr is non-zero.
 *
 * @param set The bitset to print.
 * @param cr Whether to add a new line at the end.
 */
void xanBitSet_print (XanBitSet *set, int cr);

/**
 * @brief Return the position of the first bit set in a bitset between two positions, inclusive.
 * \ingroup XanBitSet
 *
 * Find the position of the first bit that is set (i.e., 1) in @c set, starting at @c start and ending at @c end inclusive. It is assumed that @c start will be less than or equal to @c end.
 *
 * @param set The bitset to consider.
 * @param start The position to start looking.
 * @param end The position to finish looking.
 * @return The position of the first bit that is set, or -1 if no bit is set within the range.
 */
int xanBitSet_getFirstBitSetInRange(XanBitSet *set, size_t start, size_t end);

/**
 * @brief Return the position of the first bit unset in a bitset between two positions, inclusive.
 * \ingroup XanBitSet
 *
 * Find the position of the first bit that is unset (i.e., 0) in @c set, starting at @c start and ending at @c end inclusive. It is assumed that @c start will be less than or equal to @c end.
 *
 * @param set The bitset to consider.
 * @param start The position to start looking.
 * @param end The position to finish looking.
 * @return The position of the first bit that is unset, or -1 if no bit is unset within the range.
 */
int xanBitSet_getFirstBitUnsetInRange(XanBitSet *set, size_t start, size_t end);

/**
 * @brief Return the length of the bit set (i.e. the number of bits stored).
 * @ingroup XanBitSet
 *
 * Get the number of bits stored by @c set.
 *
 * @param set The bitset to get the length of.
 * @return The number of bits stored by @c set.
 */
size_t xanBitSet_length(XanBitSet *set);

/**
 * @brief Return the capacity of the bit set (i.e. the number of bytes allocated).
 * @ingroup XanBitSet
 *
 * Get the capacity of @c set in bytes.
 *
 * @param set The bitset to get the capacity of.
 * @return The capacity of @c set in bytes.
 */
size_t xanBitSet_capacity(XanBitSet *set);

void libxanCompilationFlags (char *buffer, int bufferLength);

void libxanCompilationTime (char *buffer, int bufferLength);

char * libxanVersion ();

#ifdef __cplusplus
};
#endif

#endif
