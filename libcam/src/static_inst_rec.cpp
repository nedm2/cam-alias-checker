
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

#include "static_inst_rec.h"
#include "StaticLoopRec.h"
#include <assert.h>
#include <list>
#include <iterator>

//MemSetEntry::MemSetEntry() : base(0), stride(0), length(0), start(0), end(0) {}
//
//MemSetEntry::MemSetEntry(uintptr_t b, intptr_t s, uint64_t l, uint64_t st, uint64_t e)
//    : base(b), stride(s), length(l), start(st), end(e) {}

//uintptr_t MemSetEntry::getBase() { return base; }
//intptr_t MemSetEntry::getStride() { return stride; }
//uintptr_t MemSetEntry::getLength() { return length; }
//uintptr_t MemSetEntry::getStart() { return start; }
//uintptr_t MemSetEntry::getEnd() {return end; }
//uint64_t MemSetEntry::getIterationNumber() { return iterationNumber; }
//uintptr_t MemSetEntry::getEffectiveInstrID() { return effectiveInstrID; }
//uintptr_t MemSetEntry::getLast() {return base + ((end - start)*stride); }
//void MemSetEntry::setBase(uintptr_t x) { base = x; }
//void MemSetEntry::setStride(intptr_t x) { stride = x; }
//void MemSetEntry::setLength(uint64_t x) { length = x; }
//void MemSetEntry::setStart(uint64_t x) { start = x; }
//void MemSetEntry::setEnd(uint64_t x) { end = x; }
//void MemSetEntry::incEnd() { end++; }
//void MemSetEntry::setIterationNumber(uint64_t iternum) { iterationNumber = iternum; }
//void MemSetEntry::setEffectiveInstrID(uintptr_t instrID) { effectiveInstrID = instrID; }

//uintptr_t MemSetEntry::getAccessLower(uint64_t n){
//  return base + n*stride;
//}
//
//uintptr_t MemSetEntry::getAccessUpper(uint64_t n){
//  return base + n*stride + length - 1;
//}

//bool MemSetEntry::isAligned(){
//  return base%length == 0;
//}

//uintptr_t MemSetEntry::getUpperExtent(){
//  if(stride >= 0)
//    return base + stride*(end - start) + length - 1;
//  else
//    return base + length - 1;
//}
//
//uintptr_t MemSetEntry::getLowerExtent(){
//  if(stride >= 0)
//    return base;
//  else
//    return base + stride*(end - start);
//}

//uintptr_t MemSetEntry::getNumInstances(){
//  return end - start + 1;
//}

uintptr_t MemSetEntry::prediction(){
  return base + stride*getNumInstances();
}

MemSetEntry MemSetEntry::splitOffEnd(uint64_t n){
  uint64_t remaining = getNumInstances() - n;
  MemSetEntry newEntry(base + remaining*stride, stride, length, start + remaining, end);
  end -= n;
  return newEntry;
}

MemSetEntry MemSetEntry::getSlice(uint64_t s, uint64_t e){
  assert(s >= start && e <= end && s <= e);
  return MemSetEntry(base + (s - start)*stride, stride, length, s, e);
}

//MemSetEntry MemSetEntry::getNormalised(){
//  if(stride < 0)
//    return MemSetEntry(getLast(), abs(getStride()), getLength(), getStart(), getEnd());
//  else
//    return *this;
//}

MemSetEntry::interval MemSetEntry::toInterval(MemSetEntry& entry){
  return MemSetEntry::interval(entry.getLowerExtent(), entry.getUpperExtent(), &entry);
}

uint64_t MemSetEntry::getFirstDynamicInstanceFromAddress(uintptr_t addr){
  if (stride == 0)
    return start;
  else 
    return start + ((addr - base)/stride);
}

uint64_t MemSetEntry::getLastDynamicInstanceFromAddress(uintptr_t addr){
  if (stride == 0)
    return end;
  else 
    return start + ((addr - base)/stride);
}

bool MemSetEntry::isTrivialPattern(){
  return (getNumInstances() == 2 && abs(stride) > maxStride);
}

MemSetEntry MemSetEntry::splitTrivial(){
  MemSetEntry remainder = MemSetEntry(base + stride, stride, length, start+1, end);
  end = start;
  return remainder;
}

vector<MemSetEntry> MemSetEntry::getSplitPattern(){
  vector<MemSetEntry> splits;
  for(uint64_t i = 0; i < getNumInstances(); i++)
    splits.push_back(MemSetEntry(getAccessLower(i), 0, length, start + i, start + i));
  return splits;
}

void MemSetEntry::printWithIterInfo(ostream& os){
  os << "[" << base << " " << stride << " " << length << " " << start << " " 
       << end << " (" << iterationNumber << "," << effectiveInstrID << ")] ";
}

ostream& operator<<(ostream& os, const MemSetEntry& e){
  return os << "[" << e.base << " " << e.stride << " " << e.length << " " << e.start << " " << e.end << "] ";
}


/**
 * MemSet
**/

MemSet MemSet::getInvocationSliceWithIterTags(uintptr_t instrID, InvocationGroupCfc &invGroup, uint64_t invNum, CallTraceLoopInvocationGroup& invocCallTrace){
  MemSet slice;
  uint64_t memsetRemainingInstances = outstandingInstances;
  if(invGroup.containsInstruction(instrID)){ /* This instruction is a normal loop instruction */
    //LoopInstrGroup& lig = invGroup[instrID];
    //for(auto ligIter = lig.ligIteratorBegin(); ligIter != lig.ligIteratorEnd(); ligIter = lig.ligIteratorNext(ligIter)){
    for(auto ii = invGroup.iterationIteratorBegin(); ii != invGroup.iterationIteratorEnd(); ii = invGroup.iterationIteratorNext(ii)){
      /* Accumulate MemSetEntries until we have enough to make up this LoopInstrRec */
      uint64_t ligInstances = invGroup.getNumInstancesFromII(ii, instrID);
      uint64_t ligInstancesConsumed = 0;
      while(ligInstancesConsumed < ligInstances){
        uint64_t startInstance = sliceIterator->getEnd() - memsetRemainingInstances + 1;
        uint64_t endInstance;
        if (memsetRemainingInstances > (ligInstances - ligInstancesConsumed))
          endInstance = startInstance + ligInstances - ligInstancesConsumed - 1;
        else
          endInstance = sliceIterator->getEnd();
        uint64_t instancesConsumed = endInstance - startInstance + 1;
        MemSetEntry newEntry = sliceIterator->getSlice(startInstance, endInstance);
        newEntry.setIterationNumber(invGroup.getIterationNumberFromII(ii, instrID));
        newEntry.setEffectiveInstrID(instrID);
        slice.push_back(newEntry);
        ligInstancesConsumed += instancesConsumed;
        memsetRemainingInstances -= instancesConsumed;
        if(memsetRemainingInstances == 0){
          sliceIterator++;
          if(sliceIterator != this->end())
            memsetRemainingInstances = sliceIterator->getNumInstances();
        }
      }
      assert(ligInstancesConsumed == ligInstances);
    }
  }
  else{ /* This instruction is a sub instruction (in a function called from the loop) */
    for(  auto cii = invocCallTrace.callInstanceIteratorBegin(instrID); 
          cii != invocCallTrace.callInstanceIteratorEnd(instrID); 
          cii = invocCallTrace.callInstanceIteratorNext(cii)){
      uint64_t iter = invGroup.getIterationNumber(invocCallTrace.getCallIDFromCII(cii), invocCallTrace.getCallInstanceFromCII(cii));
      uintptr_t effectiveInstrID = invocCallTrace.getCallIDFromCII(cii);
      uint64_t ctInstances = invocCallTrace.getNumInstancesFromCII(cii); /* Number of sub instruction instances for this call instance */
      uint64_t ctInstancesConsumed = 0;
      while(ctInstancesConsumed < ctInstances){
        uint64_t startInstance = sliceIterator->getEnd() - memsetRemainingInstances + 1;
        uint64_t endInstance;
        if (memsetRemainingInstances > (ctInstances - ctInstancesConsumed))
          endInstance = startInstance + ctInstances - ctInstancesConsumed - 1;
        else
          endInstance = sliceIterator->getEnd();
        uint64_t instancesConsumed = endInstance - startInstance + 1;
        MemSetEntry newEntry = sliceIterator->getSlice(startInstance, endInstance);
        newEntry.setIterationNumber(iter);
        newEntry.setEffectiveInstrID(effectiveInstrID);
        slice.push_back(newEntry);
        ctInstancesConsumed += instancesConsumed;
        memsetRemainingInstances -= instancesConsumed;
        if(memsetRemainingInstances == 0){
          sliceIterator++;
          if(sliceIterator != this->end())
            memsetRemainingInstances = sliceIterator->getNumInstances();
        }
      }
      assert(ctInstancesConsumed == ctInstances);
    }
  }
  outstandingInstances = memsetRemainingInstances;
  return slice;
}


void MemSet::initialiseSliceIterator(){
  sliceIterator = this->begin();
  if(this->empty())
    outstandingInstances = 0;
  else
    outstandingInstances = sliceIterator->getNumInstances();
}

void MemSet::splitLargeStrides(){
  MemSet newMemSet;
  for(auto i = this->begin(); i != this->end(); i++){
    if (i->getNumInstances() == 2 && abs(i->getStride()) > MemSetEntry::maxStride){
      newMemSet.push_back(MemSetEntry(i->getBase(), 0, i->getLength(), i->getStart(), i->getStart()));
      newMemSet.push_back(MemSetEntry(i->getBase() + i->getStride(), 0, i->getLength(), i->getEnd(), i->getEnd()));
    }
    else
      newMemSet.push_back(*i);
  }
  (*this) = newMemSet;
}

void MemSet::printWithIterInfo(ostream& os){
  os << size() << " ";
  for(auto i = begin(); i != end(); i++)
    i->printWithIterInfo(os);
}

ostream& operator<<(ostream& os, const MemSet& memset){
  os << memset.size() << " ";
  for(MemSet::const_iterator i = memset.begin(); i != memset.end(); i++)
    os << *i;
  return os;
}

StaticInstRec::StaticInstRec()
  : readTree(NULL), writeTree(NULL), readSetBuilt(false), writeSetBuilt(false) {}

StaticInstRec::~StaticInstRec(){
  delete readTree;
  delete writeTree;
}

void StaticInstRec::setReadSet(MemSet r) { readSet = r; readSetBuilt = true; }
void StaticInstRec::setWriteSet(MemSet w) { writeSet = w; writeSetBuilt = true; }
MemSet &StaticInstRec::getReadSet() { return readSet; }
MemSet &StaticInstRec::getWriteSet() { return writeSet; }
bool StaticInstRec::hasReadSet() { return readSetBuilt; }
bool StaticInstRec::hasWriteSet() { return writeSetBuilt; }

bool StaticInstRec::hasReadTree() { return readTree != NULL; }
bool StaticInstRec::hasWriteTree() { return writeTree != NULL; }
MemSetEntry::intervalTree *StaticInstRec::getReadTree() { return readTree; }
MemSetEntry::intervalTree *StaticInstRec::getWriteTree() { return writeTree; }

StaticInstRec StaticInstRec::getInvocationSliceWithIterTags(uintptr_t instrID, InvocationGroupCfc &invGroup, uint64_t invNum, CallTraceLoopInvocationGroup& invCallTraces){
  StaticInstRec slice;
  if(readSet.size() > 0)
    slice.setReadSet(readSet.getInvocationSliceWithIterTags(instrID, invGroup, invNum, invCallTraces));
  if(writeSet.size() > 0)
    slice.setWriteSet(writeSet.getInvocationSliceWithIterTags(instrID, invGroup, invNum, invCallTraces));
  return slice;
}

void StaticInstRec::initialiseSliceIterators(){
  readSet.initialiseSliceIterator();
  writeSet.initialiseSliceIterator();
}

void StaticInstRec::splitLargeStrides(){
  readSet.splitLargeStrides();
  writeSet.splitLargeStrides();
}

MemSetEntry::intervalTree *StaticInstRec::buildIntervalTree(MemSet &memset){
  MemSetEntry::intervalVector intervals;
  transform(memset.begin(), memset.end(), back_inserter(intervals), MemSetEntry::toInterval);
  return new MemSetEntry::intervalTree(intervals);
}

void StaticInstRec::buildReadTree(){
  readTree = buildIntervalTree(readSet);
}

void StaticInstRec::buildWriteTree(){
  writeTree = buildIntervalTree(writeSet);
}

void StaticInstRec::buildIntervalTrees(){
  readTree = buildIntervalTree(readSet);
  writeTree = buildIntervalTree(writeSet);
}

void StaticInstRec::printWithIterInfo(ostream& os){
  readSet.printWithIterInfo(os);
  os << endl;
  writeSet.printWithIterInfo(os);
  os << endl;
}

ostream& operator<<(ostream& os, const StaticInstRec& rec){
  return os << rec.readSet << endl << rec.writeSet << endl;
}

ostream& operator<<(ostream& os, const MemoryTrace& trace){
  for(MemoryTrace::const_iterator i = trace.begin(); i != trace.end(); i++)
    os << i->first << endl << i->second;
  return os;
}

StaticInstRec MemoryTrace::getInvocationSliceWithIterTags(InvocationGroupCfc &invGroup, uint64_t invNum, CallTraceLoopInvocationGroup& invCallTraces, uintptr_t instrID){
  return (*this)[instrID].getInvocationSliceWithIterTags(instrID, invGroup, invNum, invCallTraces);
}

bool MemoryTrace::hasStaticInstRec(uintptr_t instrID){
  return (this->find(instrID) != this->end());
}


void MemoryTrace::initialiseSliceIterators(){
  for (auto mti = this->begin(); mti != this->end(); mti++){
    mti->second.initialiseSliceIterators();
  }
}

void MemoryTrace::splitLargeStrides(){
  for (auto mti = this->begin(); mti != this->end(); mti++){
    mti->second.splitLargeStrides();
  }
}

void MemoryTrace::printWithIterInfo(ostream& os){
  for (auto mti = this->begin(); mti != this->end(); mti++){
    os << mti->first << endl;
    mti->second.printWithIterInfo(os);;
  }
}
