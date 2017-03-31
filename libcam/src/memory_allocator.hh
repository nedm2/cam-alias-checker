/*
 * Copyright (C) 2014  Simone Campanoni, Timothy M Jones
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

#ifndef CAM_MEMORY_ALLOCATOR_HH
#define CAM_MEMORY_ALLOCATOR_HH

#include "config.h"

#include <bzlib.h>
#include <xanlib.h>


/**
 * Base class for keeping track of memory allocation.
 **/
class BaseMemoryAllocator
{
public:
  JITUINT64 memUsed;     /**< An estimate of the amount of memory used. */
  JITUINT64 maxMemUsed;  /**< An estimate of the amount of memory used. */

  BaseMemoryAllocator()
    :memUsed(0), maxMemUsed(0)
  {
  }

  virtual ~BaseMemoryAllocator() {}

  /* Allocating and freeing memory via malloc, free, new and delete. */
  virtual void *allocMem(size_t size) = 0;
  virtual void freeMem(void *mem) = 0;
  template<class C, typename... Arguments> C *newMem(Arguments... params) {}
  template<class C> void deleteMem(C *mem) {};

  /* Record hash table manipulation. */
  virtual XanHashTable *allocHashTable(unsigned int length = 11) = 0;
  virtual XanHashTable *allocHashTable(unsigned int (*hashFunction)(void *element), int (*equalsFunction)(void *key1, void *key2)) = 0;
  virtual void freeHashTable(XanHashTable *table) = 0;
  virtual void freeHashTableAndData(XanHashTable *table) = 0;
  virtual void freeHashTableAndKey(XanHashTable *table) = 0;
  virtual void freeHashTableKeyAndData(XanHashTable *table) = 0;
  virtual void hashTableInsert(XanHashTable *table, void *key, void *element) = 0;
  virtual void hashTableRemoveItem(XanHashTable *table, XanHashTableItem *item) = 0;
  virtual void hashTableEmptyOutTable(XanHashTable *table) = 0;
  virtual void hashTableEmptyOutTableFreeData(XanHashTable *table) = 0;

  /* Record bit set manipulation. */
  virtual XanBitSet *allocBitSet(size_t length) = 0;
  virtual void freeBitSet(XanBitSet *bitset) = 0;
  virtual void setBit(XanBitSet *bitset, size_t pos) = 0;

  /* Record stack manipulation. */
  virtual XanStack *allocStack(void) = 0;
  virtual void freeStack(XanStack *stack) = 0;
  virtual void stackPush(XanStack *stack, void *element) = 0;
  virtual void *stackPop(XanStack *stack) = 0;
  virtual XanList *stackToList(XanStack *stack) = 0;

  /* Record list manipulation. */
  virtual XanList *allocList(void) = 0;
  virtual void freeList(XanList *list) = 0;
  virtual void listAppend(XanList *list, void *element) = 0;
  virtual void listDeleteItem(XanList *list, XanListItem *item) = 0;
  virtual void listEmptyOutList(XanList *list) = 0;
};


/**
 * Dummy class for keeping track of memory allocation.  Doesn't actually do any
 * book-keeping, but provides an implementation that can be swapped for a
 * proper memory allocator (such as one below) using a compiler flag.
 **/
class DummyMemoryAllocator : public BaseMemoryAllocator
{
public:
  virtual ~DummyMemoryAllocator() {}

  /* Allocating and freeing memory via malloc, free, new and delete. */
  virtual void *
  allocMem(size_t size)
  {
    return malloc(size);
  }

  virtual void
  freeMem(void *mem)
  {
    free(mem);
  }

  template<class C, typename... Arguments>
  C *
  newMem(Arguments... params)
  {
    return new C(params...);
  }

  template<class C>
  void
  deleteMem(C *mem)
  {
    delete mem;
  }

  /* Record hash table manipulation. */
  virtual XanHashTable *
  allocHashTable(unsigned int length = 11)
  {
    return xanHashTable_new(length, JITFALSE, malloc, realloc, free, NULL, NULL);
  }

  virtual XanHashTable *
  allocHashTable(unsigned int (*hashFunction)(void *element), int (*equalsFunction)(void *key1, void *key2))
  {
    return xanHashTable_new(11, JITFALSE, malloc, realloc, free, hashFunction, equalsFunction);
  }

  virtual void
  freeHashTable(XanHashTable *table)
  {
    xanHashTable_destroyTable(table);
  }

  virtual void
  freeHashTableAndData(XanHashTable *table)
  {
    xanHashTable_destroyTableAndData(table);
  }

  virtual void
  freeHashTableAndKey(XanHashTable *table)
  {
    xanHashTable_destroyTableAndKey(table);
  }

  virtual void
  freeHashTableKeyAndData(XanHashTable *table)
  {
    xanHashTable_destroyTableDataAndKey(table);
  }

  virtual void
  hashTableInsert(XanHashTable *table, void *key, void *element)
  {
    xanHashTable_insert(table, key, element);
  }

  virtual void
  hashTableRemoveItem(XanHashTable *table, XanHashTableItem *item)
  {
    xanHashTable_removeItem(table, item);
  }

  virtual void
  hashTableEmptyOutTable(XanHashTable *table)
  {
    xanHashTable_emptyOutTable(table);
  }

  virtual void
  hashTableEmptyOutTableFreeData(XanHashTable *table)
  {
    xanHashTable_deleteAndFreeElements(table);
  }

  /* Record bit set manipulation. */
  virtual XanBitSet *
  allocBitSet(size_t length)
  {
    return xanBitSet_new(length);
  }

  virtual void
  freeBitSet(XanBitSet *bitset)
  {
    xanBitSet_free(bitset);
  }

  virtual void 
  setBit(XanBitSet *bitset, size_t pos)
  {
    xanBitSet_setBit(bitset, pos);
  }

  /* Record stack manipulation. */
  virtual XanStack *
  allocStack(void)
  {
    return xanStack_new(malloc, free, NULL);
  }

  virtual void
  freeStack(XanStack *stack)
  {
    xanStack_destroyStack(stack);
  }

  virtual void
  stackPush(XanStack *stack, void *element)
  {
    xanStack_push(stack, element);
  }

  virtual void *
  stackPop(XanStack *stack)
  {
    return xanStack_pop(stack);
  }

  virtual XanList *
  stackToList(XanStack *stack)
  {
    return xanStack_toList(stack);
  }

  /* Record list manipulation. */
  virtual XanList *
  allocList(void)
  {
    return xanList_new(malloc, free, NULL);
  }

  virtual void
  freeList(XanList *list)
  {
    xanList_destroyList(list);
  }

  virtual void
  listAppend(XanList *list, void *element)
  {
    xanList_append(list, element);
  }

  virtual void
  listDeleteItem(XanList *list, XanListItem *item)
  {
    xanList_deleteItem(list, item);
  }

  virtual void
  listEmptyOutList(XanList *list)
  {
    xanList_emptyOutList(list);
  }
};


/**
 * Deal with allocating memory and keeping track of it so that we have an
 * estimate of current memory consumption for part of the analysis.
 **/
class MemoryAllocator : public BaseMemoryAllocator
{
public:
  virtual ~MemoryAllocator() {}

protected:
  /* Record an allocation or freeing of memory. */
  virtual void allocatedMem(void *mem, size_t size, int type);
  virtual void freedMem(void *mem, size_t size);

public:
  //uint64_t newMemAllocations;
  //MemoryAllocator() : newMemAllocations(0) {}

  /* Find out about memory used. */
  virtual JITUINT64
  getMemUsed(void) __attribute__ ((used))
  {
    return memUsed;
  }

  /* Find out about memory used. */
  virtual JITUINT64
  getMaxMemUsed(void) __attribute__ ((used))
  {
    return maxMemUsed;
  }

  /* Allocate and free some memory using malloc and free. */
  virtual void *allocMem(size_t size);
  virtual void freeMem(void *mem);

  /* Allocate some memory using new. */
  template<class C, typename... Arguments>
  C *
  newMem(Arguments... params)
  {
    C *mem = new C(params...);
    allocatedMem(mem, sizeof(C), 0);
    return mem;
  }

  /* Free some memory using delete. */
  template<class C>
  void
  deleteMem(C *mem)
  {
    delete mem;
    freedMem(mem, sizeof(C));
  }

  /* Record hash table manipulation. */
  virtual XanHashTable *allocHashTable(unsigned int length = 11);
  virtual XanHashTable *allocHashTable(unsigned int (*hashFunction)(void *element), int (*equalsFunction)(void *key1, void *key2));
  virtual void freeHashTable(XanHashTable *table);
  virtual void freeHashTableAndData(XanHashTable *table);
  virtual void freeHashTableAndKey(XanHashTable *table);
  virtual void freeHashTableKeyAndData(XanHashTable *table);
  virtual void hashTableInsert(XanHashTable *table, void *key, void *element);
  virtual void hashTableRemoveItem(XanHashTable *table, XanHashTableItem *item);
  virtual void hashTableEmptyOutTable(XanHashTable *table);
  virtual void hashTableEmptyOutTableFreeData(XanHashTable *table);

  /* Record bit set manipulation. */
  virtual XanBitSet *allocBitSet(size_t length);
  virtual void freeBitSet(XanBitSet *bitset);
  virtual void setBit(XanBitSet *bitset, size_t pos);

  /* Record stack manipulation. */
  virtual XanStack *allocStack(void);
  virtual void freeStack(XanStack *stack);
  virtual void stackPush(XanStack *stack, void *element);
  virtual void *stackPop(XanStack *stack);
  virtual XanList *stackToList(XanStack *stack);

  /* Record list manipulation. */
  virtual XanList *allocList(void);
  virtual void freeList(XanList *list);
  virtual void listAppend(XanList *list, void *element);
  virtual void listDeleteItem(XanList *list, XanListItem *item);
  virtual void listEmptyOutList(XanList *list);
};


/**
 * Deal with allocating memory and keeping track of it so that we have an
 * estimate of current memory consumption for part of the analysis.  Includes
 * debugging information to see which memory has not been freed at any point
 * in time.
 **/
class DebugMemoryAllocator : public MemoryAllocator
{
protected:
  JITNINT allocTypes;            /**< Different types of memory allocated. */
  JITUINT64 *memAllocatedByType; /**< An estimate of the amount of memory allocated by type. */
  XanHashTable *memAllocations;  /**< Keeps track of the size and type of each memory allocation. */

  /* Record an allocation or freeing of memory. */
  virtual void allocatedMem(void *mem, size_t size, int type);
  virtual void freedMem(void *mem, size_t size);
  virtual void printMemAllocations(void);
  virtual void printMemAllocated(void);
  virtual void printMemAllocatedOfType(int type);

  /* Deal with hash tables changing size. */
  virtual void hashTableRecordAllItemsFreed(XanHashTable *table);
  virtual void hashTableRecordAllItemsAllocated(XanHashTable *table);

public:
  DebugMemoryAllocator();
  virtual ~DebugMemoryAllocator();

  /* Record hash table manipulation. */
  virtual void freeHashTable(XanHashTable *table);
  virtual void freeHashTableAndData(XanHashTable *table);
  virtual void freeHashTableAndKey(XanHashTable *table);
  virtual void freeHashTableKeyAndData(XanHashTable *table);
  virtual void hashTableInsert(XanHashTable *table, void *key, void *element);
  virtual void hashTableEmptyOutTable(XanHashTable *table);

  /* Record stack manipulation. */
  virtual void freeStack(XanStack *stack);
  virtual void stackPush(XanStack *stack, void *element);
  virtual void *stackPop(XanStack *stack);
  virtual XanList *stackToList(XanStack *stack);

  /* Record list manipulation. */
  virtual void freeList(XanList *list);
  virtual void listAppend(XanList *list, void *element);
  virtual void listEmptyOutList(XanList *list);

  virtual void setBit(XanBitSet *bitset, size_t pos);

  /* Check whether memory is allocated. */
  virtual bool isMemoryAllocated(void *mem);
};


/**
 * The memory allocator changes depending on whether memory debugging is used.
 **/
//#define DEBUG_MEMUSE
#if defined(DEBUG_MEMUSE)
class CamMemoryAllocator : public DebugMemoryAllocator
#elif defined(TRACK_MEM_ALLOCATIONS)
class CamMemoryAllocator : public MemoryAllocator
#else
class CamMemoryAllocator : public DummyMemoryAllocator
#endif
{
};


#endif  /* CAM_MEMORY_ALLOCATOR_HH */
