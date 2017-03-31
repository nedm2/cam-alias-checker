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
#include <assert.h>
#include <xanlib.h>

#include <algorithm>
#include <iostream>

#include "cam_system.h"
#include "memory_allocator.hh"


using namespace std;


/**
 * Convert from an integer to pointer and back.
 **/
#define intToPtr(val) ((void *)(val))
#define uintToPtr(val) ((void *)(val))
#define ptrToInt(ptr) ((JITNINT)(ptr))
#define ptrToUint(ptr) ((JITNUINT)(ptr))

#define BITSET_BITS_IN_A_WORD ( 8 * sizeof(size_t) )
#define BITSET_WORDS_NECESSARY(length) \
  ( ((length) + (BITSET_BITS_IN_A_WORD - 1)) / BITSET_BITS_IN_A_WORD )
#define BITSET_BYTES_NECESSARY(v) 4*BITSET_WORDS_NECESSARY(v)


/**
 * Record an allocation of memory.
 **/
void
MemoryAllocator::allocatedMem(void *mem, size_t size, int type)
{
  memUsed += size;
  maxMemUsed = max(maxMemUsed, memUsed);
}


/**
 * Record a deallocation of memory.
 **/
void
MemoryAllocator::freedMem(void *mem, size_t size)
{
  assert(size > 0);
  assert(memUsed >= size);
  memUsed -= size;
}


/**
 * Allocate some memory using malloc.
 **/
void *
MemoryAllocator::allocMem(size_t size)
{
  size += sizeof(size_t);
  void *mem = malloc(size);
  *(size_t *)mem = size;
  allocatedMem(mem, size, 1); 
  //newMemAllocations += size;
  return (void *)((char *)mem + sizeof(size_t));
}


/**
 * Free some memory using free.
 **/
void
MemoryAllocator::freeMem(void *mem)
{
  mem = (void *)((char *)mem - sizeof(size_t));
  size_t size = *(size_t *)mem;
  free(mem);
  freedMem(mem, size);
  //newMemAllocations -= size;
}


/**
 * Allocate a new hash table.
 **/
XanHashTable *
MemoryAllocator::allocHashTable(unsigned int length)
{
  XanHashTable *table = xanHashTable_new(length, JITFALSE, malloc, realloc, free, NULL, NULL);
  allocatedMem(table, sizeof(XanHashTable), 2);
  //newMemAllocations += sizeof(XanHashTable);
  return table;
}

XanHashTable *
MemoryAllocator::allocHashTable(unsigned int (*hashFunction)(void *element), int (*equalsFunction)(void *key1, void *key2))
{
  XanHashTable *table = xanHashTable_new(11, JITFALSE, malloc, realloc, free, hashFunction, equalsFunction);
  allocatedMem(table, sizeof(XanHashTable), 3);
  //newMemAllocations += sizeof(XanHashTable);
  return table;
}


/**
 * Free a hash table.
 **/
void
MemoryAllocator::freeHashTable(XanHashTable *table)
{
  int numElements = xanHashTable_elementsInside(table);
  if (numElements > 0) {
    freedMem(NULL, numElements * sizeof(XanHashTableItem));
    //newMemAllocations -= numElements * sizeof(XanHashTableItem);
  }
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
  //newMemAllocations -= sizeof(XanHashTable);
}

void
MemoryAllocator::freeHashTableAndData(XanHashTable *table)
{
  XanHashTableItem *item;
  while ((item = xanHashTable_first(table)) != NULL) {
    freeMem(item->element);
    hashTableRemoveItem(table, item);
  }
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
  //newMemAllocations -= sizeof(XanHashTable);
}

void
MemoryAllocator::freeHashTableAndKey(XanHashTable *table)
{
  XanHashTableItem *item;
  while ((item = xanHashTable_first(table)) != NULL) {
    freeMem(item->elementID);
    hashTableRemoveItem(table, item);
  }
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
  //newMemAllocations -= sizeof(XanHashTable);
}

void
MemoryAllocator::freeHashTableKeyAndData(XanHashTable *table)
{
  XanHashTableItem *item;
  while ((item = xanHashTable_first(table)) != NULL) {
    freeMem(item->elementID);
    freeMem(item->element);
    hashTableRemoveItem(table, item);
  }
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
  //newMemAllocations -= sizeof(XanHashTable);
}


/**
 * Insert into a hash table.
 **/
void
MemoryAllocator::hashTableInsert(XanHashTable *table, void *key, void *element)
{
  xanHashTable_insert(table, key, element);
  allocatedMem(NULL, sizeof(XanHashTableItem), 4);
  //newMemAllocations += sizeof(XanHashTableItem);
}


/**
 * Remove an item from a hash table.
 **/
void
MemoryAllocator::hashTableRemoveItem(XanHashTable *table, XanHashTableItem *item)
{
  xanHashTable_removeItem(table, item);
  freedMem(item, sizeof(XanHashTableItem));
  //newMemAllocations -= sizeof(XanHashTableItem);
}


/**
 * Empty a hash table.
 **/
void
MemoryAllocator::hashTableEmptyOutTable(XanHashTable *table)
{
  int size = xanHashTable_elementsInside(table);
  if (size > 0) {
    freedMem(NULL, size * sizeof(XanHashTableItem));
    //newMemAllocations -= size * sizeof(XanHashTableItem);
  }
  xanHashTable_emptyOutTable(table);
}

void
MemoryAllocator::hashTableEmptyOutTableFreeData(XanHashTable *table)
{
  XanHashTableItem *item;
  while ((item = xanHashTable_first(table)) != NULL) {
    freeMem(item->element);
    hashTableRemoveItem(table, item);
  }
}


/**
 * Allocate a new stack.
 **/
XanStack *
MemoryAllocator::allocStack(void)
{
  XanStack *stack = xanStack_new(malloc, free, NULL);
  allocatedMem(stack, sizeof(XanStack), 5);
  return stack;
}


/**
 * Free a stack.
 **/
void
MemoryAllocator::freeStack(XanStack *stack)
{
  int size = xanStack_getSize(stack);
  if (size > 0) {
    freedMem(NULL, size * sizeof(XanListItem));
  }
  xanStack_destroyStack(stack);
  freedMem(stack, sizeof(XanStack));
}


/**
 * Push onto a stack.
 **/
void
MemoryAllocator::stackPush(XanStack *stack, void *element)
{
  xanStack_push(stack, element);
  allocatedMem(NULL, sizeof(XanListItem), 6);
}


/**
 * Pop from a stack.
 **/
void *
MemoryAllocator::stackPop(XanStack *stack)
{
  freedMem(NULL, sizeof(XanListItem));
  return xanStack_pop(stack);
}


/**
 * Convert a stack to a list.
 **/
XanList *
MemoryAllocator::stackToList(XanStack *stack)
{
  XanList *list = xanStack_toList(stack);
  int length = xanList_length(list);
  allocatedMem(list, sizeof(XanList), 7);
  if (length > 0) {
    allocatedMem(NULL, length * sizeof(XanListItem), 8);
  }
  return list;
}


/**
 * Allocate a new bit set.
 **/
XanBitSet *
MemoryAllocator::allocBitSet(size_t length)
{
  XanBitSet *bitset = xanBitSet_new(length);
  allocatedMem(bitset, sizeof(XanBitSet) + xanBitSet_capacity(bitset), 9);
  //allocatedMem(bitset, sizeof(XanBitSet) + BITSET_BYTES_NECESSARY(xanBitSet_length(bitset)), 9);
  //newMemAllocations += sizeof(XanBitSet) + BITSET_BYTES_NECESSARY(xanBitSet_length(bitset));
  return bitset;
}


/**
 * Free a bit set.
 **/
void
MemoryAllocator::freeBitSet(XanBitSet *bitset)
{
  freedMem(bitset, sizeof(XanBitSet) + xanBitSet_capacity(bitset));
  //freedMem(bitset, sizeof(XanBitSet) + BITSET_BYTES_NECESSARY(xanBitSet_length(bitset)));
  //newMemAllocations -= sizeof(XanBitSet) + BITSET_BYTES_NECESSARY(xanBitSet_length(bitset));
  xanBitSet_free(bitset);
}

/**
 * Set a bit. May expand the bitset.
 **/
void
MemoryAllocator::setBit(XanBitSet *bitset, size_t pos)
{
  size_t originalSize = xanBitSet_capacity(bitset);
  xanBitSet_setBit(bitset, pos);
  size_t newSize = xanBitSet_capacity(bitset);
  assert(newSize >= originalSize);
  if(newSize > originalSize){
    allocatedMem(NULL, newSize - originalSize, 9);
    //newMemAllocations += newSize - originalSize; 
  }
}


/**
 * Allocate a new list.
 **/
XanList *
MemoryAllocator::allocList(void)
{
  XanList *list = xanList_new(malloc, free, NULL);
  allocatedMem(list, sizeof(XanList), 10);
  return list;
}


/**
 * Free a list.
 **/
void
MemoryAllocator::freeList(XanList *list)
{
  int length = xanList_length(list);
  if (length > 0) {
    freedMem(NULL, length * sizeof(XanListItem));
  }
  xanList_destroyList(list);
  freedMem(list, sizeof(XanList));
}


/**
 * Append to a list.
 **/
void
MemoryAllocator::listAppend(XanList *list, void *element)
{
  xanList_append(list, element);
  allocatedMem(NULL, sizeof(XanListItem), 11);
}


/**
 * Remove an item from a list.
 **/
void
MemoryAllocator::listDeleteItem(XanList *list, XanListItem *item)
{
  xanList_deleteItem(list, item);
  freedMem(item, sizeof(XanListItem));
}

/**
 * Clear data from list without deallocating the list itself.
 **/
void
MemoryAllocator::listEmptyOutList(XanList *list)
{
  if(xanList_length(list) > 0){
    freedMem(list, xanList_length(list) * sizeof(XanListItem));
    xanList_emptyOutList(list);
  }
}

/**
 * Debugging memory allocator constructor.
 **/
DebugMemoryAllocator::DebugMemoryAllocator()
{
  allocTypes = 1;
  memAllocatedByType = (JITUINT64 *)calloc(1, sizeof(JITUINT64));
  memAllocations = xanHashTable_new(11, JITFALSE, malloc, realloc, free, NULL, NULL);
}


/**
 * Debugging memory allocator destructor.
 **/
DebugMemoryAllocator::~DebugMemoryAllocator()
{
  if (memUsed > 0) {
    cerr << "Still using " << memUsed << " memory" << endl;
    printMemAllocations();
    abort();
  }
  free(memAllocatedByType);
  assert(xanHashTable_elementsInside(memAllocations) == 0);
  xanHashTable_destroyTable(memAllocations);
  cerr << "Used " << maxMemUsed << " memory total" << endl;
}


/**
 * A memory allocation.
 **/
struct MemAlloc
{
  size_t size;
  int type;

  MemAlloc(size_t s, int t)
  {
    size = s;
    type = t;
  }
};


/**
 * Record an allocation of memory.
 **/
void
DebugMemoryAllocator::allocatedMem(void *mem, size_t size, int type)
{
  memUsed += size;
  maxMemUsed = max(maxMemUsed, memUsed);
  assert(!xanHashTable_lookup(memAllocations, mem));
  xanHashTable_insert(memAllocations, mem, new MemAlloc(size, type));
  if (type >= allocTypes) {
    JITNINT i, oldTypes = allocTypes;
    JITUINT64 *oldAllocs = memAllocatedByType;
    allocTypes = type + 1;
    memAllocatedByType = (JITUINT64 *)malloc(allocTypes * sizeof(JITUINT64));
    for (i = 0; i < allocTypes; ++i) {
      if (i < oldTypes) {
        memAllocatedByType[i] = oldAllocs[i];
      } else {
        memAllocatedByType[i] = 0;
      }
    }
    free(oldAllocs);
  }
  memAllocatedByType[type] += size;
}


/**
 * Record a deallocation of memory.
 **/
void
DebugMemoryAllocator::freedMem(void *mem, size_t size)
{
  MemAlloc *allocation = (MemAlloc *)xanHashTable_lookup(memAllocations, mem);
  xanHashTable_removeElement(memAllocations, mem);
  assert(allocation);
  assert(allocation->size == size);
  assert(memAllocatedByType[allocation->type] >= allocation->size);
  memAllocatedByType[allocation->type] -= allocation->size;
  assert(memUsed >= allocation->size);
  memUsed -= allocation->size;
  delete allocation;
}


/**
 * Print memory allocations by type.
 **/
void
DebugMemoryAllocator::printMemAllocations(void)
{
  JITNINT i;
  for (i = 0; i < allocTypes; ++i) {
    if (memAllocatedByType[i] > 0) {
      cerr << "Type " << i << ": " << memAllocatedByType[i] << endl;
    }
  }
}


/**
 * Print all memory allocated.
 **/
void
DebugMemoryAllocator::printMemAllocated(void)
{
  cerr << "Still " << xanHashTable_elementsInside(memAllocations) << " memory elements allocated" << endl;
  XanHashTableItem *item = xanHashTable_first(memAllocations);
  while (item) {
    MemAlloc *allocation = (MemAlloc *)item->element;
    cerr << "Allocation " << item->elementID << " size " << allocation->size << " type " << allocation->type << endl;
    item = xanHashTable_next(memAllocations, item);
  }
}

void
DebugMemoryAllocator::printMemAllocatedOfType(int type)
{
  cerr << "Still " << xanHashTable_elementsInside(memAllocations) << " memory elements allocated" << endl;
  XanHashTableItem *item = xanHashTable_first(memAllocations);
  while (item) {
    MemAlloc *allocation = (MemAlloc *)item->element;
    assert(allocation);
    if (allocation->type == type) {
      cerr << "Allocation " << item->elementID << " size " << allocation->size << " type " << allocation->type << endl;
    }
    item = xanHashTable_next(memAllocations, item);
  }
}


/**
 * Check whether memory is allocated.
 **/
bool
DebugMemoryAllocator::isMemoryAllocated(void *mem)
{
  return xanHashTable_lookupItem(memAllocations, mem) != NULL;
}



/**
 * Record all hash table items as being freed.
 **/
void
DebugMemoryAllocator::hashTableRecordAllItemsFreed(XanHashTable *table)
{
  XanHashTableItem *item = xanHashTable_first(table);
  while (item) {
    freedMem(item, sizeof(XanHashTableItem));
    item = xanHashTable_next(table, item);
  }
}


/**
 * Record all hash table items as being allocated.
 **/
void
DebugMemoryAllocator::hashTableRecordAllItemsAllocated(XanHashTable *table)
{
  XanHashTableItem *item = xanHashTable_first(table);
  while (item) {
    allocatedMem(item, sizeof(XanHashTableItem), 12);
    item = xanHashTable_next(table, item);
  }
}


/**
 * Free a hash table.
 **/
void
DebugMemoryAllocator::freeHashTable(XanHashTable *table)
{
  hashTableRecordAllItemsFreed(table);
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
}

void
DebugMemoryAllocator::freeHashTableAndData(XanHashTable *table)
{
  XanHashTableItem *item = xanHashTable_first(table);
  while (item) {
    freeMem(item->element);
    freedMem(item, sizeof(XanHashTableItem));
    item = xanHashTable_next(table, item);
  }
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
}

void
DebugMemoryAllocator::freeHashTableAndKey(XanHashTable *table)
{
  XanHashTableItem *item = xanHashTable_first(table);
  while (item) {
    freeMem(item->elementID);
    freedMem(item, sizeof(XanHashTableItem));
    item = xanHashTable_next(table, item);
  }
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
}

void
DebugMemoryAllocator::freeHashTableKeyAndData(XanHashTable *table)
{
  XanHashTableItem *item = xanHashTable_first(table);
  while (item) {
    freeMem(item->elementID);
    freeMem(item->element);
    freedMem(item, sizeof(XanHashTableItem));
    item = xanHashTable_next(table, item);
  }
  xanHashTable_destroyTable(table);
  freedMem(table, sizeof(XanHashTable));
}


/**
 * Insert into a hash table.
 **/
void
DebugMemoryAllocator::hashTableInsert(XanHashTable *table, void *key, void *element)
{
  bool expanded = false;
  if (table->currentLoadFactor >= 0.75) {
    hashTableRecordAllItemsFreed(table);
    expanded = true;
  }
  xanHashTable_insert(table, key, element);
  if (expanded) {
    hashTableRecordAllItemsAllocated(table);
  } else {
    allocatedMem(xanHashTable_lookupItem(table, key), sizeof(XanHashTableItem), 13);
  }
}


/**
 * Empty a hash table.
 **/
void
DebugMemoryAllocator::hashTableEmptyOutTable(XanHashTable *table)
{
  hashTableRecordAllItemsFreed(table);
  xanHashTable_emptyOutTable(table);
}


/**
 * Free a stack.
 **/
void
DebugMemoryAllocator::freeStack(XanStack *stack)
{
  XanListItem *item;
  while ((item = xanList_first(stack->internalList)) != NULL) {
    listDeleteItem(stack->internalList, item);
  }
  xanStack_destroyStack(stack);
  freedMem(stack, sizeof(XanStack));
}


/**
 * Push onto a stack.
 **/
void
DebugMemoryAllocator::stackPush(XanStack *stack, void *element)
{
  xanStack_push(stack, element);
  allocatedMem(xanList_first(stack->internalList), sizeof(XanListItem), 14);
}


/**
 * Pop from a stack.
 **/
void *
DebugMemoryAllocator::stackPop(XanStack *stack)
{
  freedMem(xanList_first(stack->internalList), sizeof(XanListItem));
  return xanStack_pop(stack);
}


/**
 * Convert a stack to a list.
 **/
XanList *
DebugMemoryAllocator::stackToList(XanStack *stack)
{
  XanList *list = xanStack_toList(stack);
  allocatedMem(list, sizeof(XanList), 15);
  XanListItem *item = xanList_first(list);
  while (item) {
    allocatedMem(item, sizeof(XanListItem), 16);
    item = item->next;
  }
  return list;
}


/**
 * Free a list.
 **/
void
DebugMemoryAllocator::freeList(XanList *list)
{
  listEmptyOutList(list);
  xanList_destroyList(list);
  freedMem(list, sizeof(XanList));
}

/**
 * Clear data from a list.
 **/
void
DebugMemoryAllocator::listEmptyOutList(XanList *list)
{
  XanListItem *item;
  while ((item = xanList_first(list)) != NULL) {
    listDeleteItem(list, item);
  }
}


/**
 * Append to a list.
 **/
void
DebugMemoryAllocator::listAppend(XanList *list, void *element)
{
  xanList_append(list, element);
  allocatedMem(xanList_last(list), sizeof(XanListItem), 17);
}

void 
DebugMemoryAllocator::setBit(XanBitSet *bitset, size_t pos)
{
  size_t originalSize = xanBitSet_capacity(bitset);
  xanBitSet_setBit(bitset, pos);
  size_t newSize = xanBitSet_capacity(bitset);
  assert(newSize >= originalSize);
  if(newSize > originalSize){
    size_t size = newSize - originalSize;
    memUsed += size;
    //newMemAllocations += size; 
    maxMemUsed = max(maxMemUsed, memUsed);
    XanHashTableItem* entry = xanHashTable_lookupItem(memAllocations, bitset);
    MemAlloc* entryMemAlloc = (MemAlloc*)entry->element;
    entryMemAlloc->size += size;
    memAllocatedByType[9] += size;
    assert(entryMemAlloc->size == sizeof(XanBitSet) + xanBitSet_capacity(bitset));
  }
}
