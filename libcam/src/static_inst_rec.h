
/*
 * Copyright (C) 2012 - 2015  Niall Murphy
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

#ifndef STATICINSTREC_H
#define STATICINSTREC_H

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include "IntervalTree.h"
#include "StaticLoopRec.h"
#include "CallTrace.h"


class MemSetEntry {
  uintptr_t base;
  intptr_t stride;
  uint64_t length;
  uint64_t start;
  uint64_t end;
  uint64_t iterationNumber;
  uintptr_t effectiveInstrID;

public:
  typedef Interval<MemSetEntry *, uint64_t> interval;
  typedef vector<interval> intervalVector;
  typedef IntervalTree<MemSetEntry *, uint64_t> intervalTree;

  MemSetEntry() : base(0), stride(0), length(0), start(0), end(0) {}

  MemSetEntry(uintptr_t b, intptr_t s, uint64_t l, uint64_t st, uint64_t e)
    : base(b), stride(s), length(l), start(st), end(e) {}

  uintptr_t getBase() { return base; }
  intptr_t getStride() { return stride; }
  uintptr_t getLength() { return length; }
  uintptr_t getStart() { return start; }
  uintptr_t getEnd() { return end; }
  uint64_t getIterationNumber() { return iterationNumber; }
  uintptr_t getEffectiveInstrID() { return effectiveInstrID; }
  uintptr_t getLast() {return base + ((end - start)*stride); }
  void setBase(uintptr_t x) { base = x; }
  void setStride(intptr_t x) { stride = x; }
  void setLength(uint64_t x) { length = x; }
  void setStart(uint64_t x) { start = x; }
  void setEnd(uint64_t x) { end = x; }
  void incEnd() { end++; }
  void setIterationNumber(uint64_t iternum) { iterationNumber = iternum; }
  void setEffectiveInstrID(uintptr_t instrID) { effectiveInstrID = instrID; }

  uintptr_t getNumInstances() {
    return end - start + 1;
  }

  uintptr_t getUpperExtent(){
    if(stride >= 0)
      return base + stride*(end - start) + length - 1;
    else
      return base + length - 1;
  }

  uintptr_t getLowerExtent(){
    if(stride >= 0)
      return base;
    else
      return base + stride*(end - start);
  }

  /* Returns the upper and lower extent of the nth access in this MemSetEntry */
  uintptr_t getAccessLower(uint64_t n) {
    return base + n*stride;
  }
  
  uintptr_t getAccessUpper(uint64_t n) {
    return base + n*stride + length - 1;
  }

  /* Returns true if an n-byte access is aligned on an n-byte boundary */
  bool isAligned(){
    return base%length == 0;
  }

  /* Return a MemSetEntry with positive stride (i.e. reverse the range if stride is negative).
   * Dynamic instance numbers of the normalised MemSetEntry will be invalid.
  */
  MemSetEntry getNormalised(){
    if(stride < 0)
      return MemSetEntry(getLast(), abs(getStride()), getLength(), getStart(), getEnd());
    else
      return *this;
  }

  /* Return the next address as predicted by the pattern */
  uintptr_t prediction();

  /* Split n instances off the end of this entry and return a new entry containing those instances */
  MemSetEntry splitOffEnd(uint64_t n);

  /* Return a new MemSetEntry spanning only the instances specified */
  MemSetEntry getSlice(uint64_t s, uint64_t e);

  /* Return true if the pattern has only 2 instances with a large stride (greater than maxStride) */
  bool isTrivialPattern();

  /* Return a vector with a MemSetEntry for each instance in the pattern */
  vector<MemSetEntry> getSplitPattern();

  /* For a trivial pattern, reduce this entry to 1 instance and return the remaining instances in a new entry */
  MemSetEntry splitTrivial();

  /* Return the first/last dynamic instance number which is based at the given address, addr must be aligned
   * and must be within the range of the MemSetEntry */
  uint64_t getFirstDynamicInstanceFromAddress(uintptr_t addr);
  uint64_t getLastDynamicInstanceFromAddress(uintptr_t addr);

  static interval toInterval(MemSetEntry& entry);

  void printWithIterInfo(ostream& os);

  friend ostream& operator<<(ostream& os, const MemSetEntry& e);

  static const intptr_t maxStride = 8;
};

class MemSet : public vector<MemSetEntry>{
  MemSet::iterator sliceIterator;
  uint64_t outstandingInstances;
public:
  //MemSet getInvocationSlice(uintptr_t instrID, InvocationGroup &invGroup, uint64_t invNum, CallTraces& invCallTraces);
  MemSet getInvocationSliceWithIterTags(uintptr_t instrID, InvocationGroupCfc &invGroup, uint64_t invNum, CallTraceLoopInvocationGroup& invCallTraces);
  void initialiseSliceIterator();

  /* For all entries with only 2 instances and a large stride, split into 2 entries */
  void splitLargeStrides();

  void printWithIterInfo(ostream& os);

  friend ostream& operator<<(ostream& os, const MemSet& memset);
};


struct StaticInstRec {
  MemSet readSet;
  MemSet writeSet;
  MemSetEntry::intervalTree *readTree;
  MemSetEntry::intervalTree *writeTree;
  bool readSetBuilt;
  bool writeSetBuilt;

  StaticInstRec();
  ~StaticInstRec();

  void setReadSet(MemSet r);
  void setWriteSet(MemSet w);
  MemSet &getReadSet();
  MemSet &getWriteSet();
  bool hasReadTree();
  bool hasWriteTree();
  bool hasReadSet();
  bool hasWriteSet();
  MemSetEntry::intervalTree *getReadTree();
  MemSetEntry::intervalTree *getWriteTree();

  void buildIntervalTrees();
  void buildReadTree();
  void buildWriteTree();

  void initialiseSliceIterators();

  /* For all entries with only 2 instances and a large stride, split into 2 entries */
  void splitLargeStrides();

  //StaticInstRec getInvocationSlice(uintptr_t instrID, InvocationGroup &invGroup, uint64_t invNum, CallTraces& invCallTraces);
  StaticInstRec getInvocationSliceWithIterTags(uintptr_t isntrID, InvocationGroupCfc &invGroup, uint64_t invNum, CallTraceLoopInvocationGroup& invCallTraces);

  void printWithIterInfo(ostream& os);

  friend ostream& operator<<(ostream& os, const StaticInstRec& loops);

private: 
  MemSetEntry::intervalTree *buildIntervalTree(MemSet &memset);
};

class MemoryTrace : public map<uintptr_t, StaticInstRec>{
public:
  /* Get the slice of memory accesses which occured during a specific invocation */
  StaticInstRec getInvocationSliceWithIterTags(InvocationGroupCfc &invGroup, uint64_t invNum, CallTraceLoopInvocationGroup& invCallTraces, uintptr_t instrID);

  void initialiseSliceIterators();

  /* For all entries with only 2 instances and a large stride, split into 2 entries */
  void splitLargeStrides();

  void printWithIterInfo(ostream& os);

  bool hasStaticInstRec(uintptr_t instrID);

  friend ostream& operator<<(ostream& os, const MemoryTrace& loops);
};


#endif
