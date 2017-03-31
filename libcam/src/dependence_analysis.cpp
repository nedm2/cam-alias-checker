
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

#include "dependence_analysis.h"
#include "parser_wrappers.h"
#include "cam_system.h"
#include "dynamic_gcd.h"
#include "TimeoutCounter.h"
#include "IntervalTree.h"
#include "MemoryTraceStreamer.h"
#include <fstream>
#include <list>
#include <set>
#include <assert.h>
#include <algorithm>
#include <xanlib.h>

/**
 * Convert from an integer to pointer and back.
 **/
#define intToPtr(val) ((void *)(val))
#define uintToPtr(val) ((void *)(val))
#define ptrToInt(ptr) ((JITNINT)(ptr))
#define ptrToUint(ptr) ((JITNUINT)(ptr))

MemSetEntry &AliasRec::getSetA() const { return setA; }
MemSetEntry &AliasRec::getSetB() const { return setB; }
uintptr_t AliasRec::getInstrA() const { return instrA; }
uintptr_t AliasRec::getInstrB() const { return instrB; }
AliasT PatternAlias::getAlias() const { return alias; }

uint64_t constant = 0;
uint64_t unknown = 0;

#if 0
bool BruteForceAlias::allAliasesAreIntraIteration(InvocationGroup &invGroup, uint64_t invocNum, CallTraces& callTraces){
  if(getSetA().getStride() == 0 || getSetB().getStride() == 0)
    constant++;
  else 
    unknown++;
  for(auto i = deps.begin(); i != deps.end(); i++){
    uint64_t aFirst = getSetA().getFirstDynamicInstanceFromAddress(get<0>(*i));
    uint64_t aLast = getSetA().getLastDynamicInstanceFromAddress(get<0>(*i));
    uint64_t bFirst = getSetB().getFirstDynamicInstanceFromAddress(get<2>(*i));
    uint64_t bLast = getSetB().getLastDynamicInstanceFromAddress(get<2>(*i));
    uint64_t aFirstIter, aLastIter, bFirstIter, bLastIter;
    if(invGroup.containsInstruction(getInstrA())){
      aFirstIter = invGroup.getIterationNumber(getInstrA(), aFirst);
      aLastIter = invGroup.getIterationNumber(getInstrA(), aLast);
    }
    else{
      pair<uintptr_t, uint64_t> rootCallFirst = callTraces.getRootCallIdAndInstance(getInstrA(), aFirst);
      pair<uintptr_t, uint64_t> rootCallLast = callTraces.getRootCallIdAndInstance(getInstrA(), aLast);
      aFirstIter = invGroup.getIterationNumber(rootCallFirst.first, rootCallFirst.second);
      aLastIter = invGroup.getIterationNumber(rootCallLast.first, rootCallLast.second);
    }
    if(invGroup.containsInstruction(getInstrB())){
      bFirstIter = invGroup.getIterationNumber(getInstrB(), bFirst);
      bLastIter = invGroup.getIterationNumber(getInstrB(), bLast);
    }
    else{
      pair<uintptr_t, uint64_t> rootCallFirst = callTraces.getRootCallIdAndInstance(getInstrB(), bFirst);
      pair<uintptr_t, uint64_t> rootCallLast = callTraces.getRootCallIdAndInstance(getInstrB(), bLast);
      bFirstIter = invGroup.getIterationNumber(rootCallFirst.first, rootCallFirst.second);
      bLastIter = invGroup.getIterationNumber(rootCallLast.first, rootCallLast.second);
    }
    if(aFirstIter != bFirstIter || aLastIter != bFirstIter || bLastIter != aFirstIter){
      return false;
    }
  }
  return true;
}

bool PatternAlias::allAliasesAreIntraIteration(InvocationGroup &invGroup, uint64_t invocNum, CallTraces& callTraces){
  uint64_t aFirst = getSetA().getFirstDynamicInstanceFromAddress(get<0>(alias));
  uint64_t aLast = getSetA().getLastDynamicInstanceFromAddress(get<0>(alias) + get<1>(alias)*(get<2>(alias)-1)); //base + stride*(count-1)
  uint64_t bFirst = getSetB().getFirstDynamicInstanceFromAddress(get<0>(alias));
  uint64_t bLast = getSetB().getLastDynamicInstanceFromAddress(get<0>(alias) + get<1>(alias)*(get<2>(alias)-1)); //base + stride*(count-1)
  if(invGroup.containsInstruction(getInstrA()) && invGroup.containsInstruction(getInstrB()) &&
      invGroup.isRangeContainedWithinSinglePattern(getInstrA(), aFirst, aLast) &&
      invGroup.isRangeContainedWithinSinglePattern(getInstrB(), bFirst, bLast)){
    uint64_t aFirstIter = invGroup.getIterationNumber(getInstrA(), aFirst);
    uint64_t aLastIter = invGroup.getIterationNumber(getInstrA(), aLast);
    uint64_t bFirstIter = invGroup.getIterationNumber(getInstrB(), bFirst);
    uint64_t bLastIter = invGroup.getIterationNumber(getInstrB(), bLast);
    if(aFirstIter == bFirstIter && aLastIter == bLastIter){
      if(get<1>(alias) == 0 && get<2>(alias) > 1 && aFirstIter != aLastIter) // If multiple accesses to same addr, alias is inter iteration
        return false;
      else
        return true;
    }
    else
      return false;
  }
  else if(invGroup.containsInstruction(getInstrA()) && invGroup.containsInstruction(getInstrB()) && 
            callTraces.isRegular(getInstrA() && callTraces.isRegular(getInstrB()))){
    pair<uintptr_t, uint64_t> aCallFirst = callTraces.getRootCallIdAndInstance(getInstrA(), aFirst);
    pair<uintptr_t, uint64_t> aCallLast = callTraces.getRootCallIdAndInstance(getInstrA(), aLast);
    pair<uintptr_t, uint64_t> bCallFirst = callTraces.getRootCallIdAndInstance(getInstrB(), bFirst);
    pair<uintptr_t, uint64_t> bCallLast = callTraces.getRootCallIdAndInstance(getInstrB(), bLast);
    assert(aCallFirst.first == aCallLast.first && bCallFirst.first == bCallLast.first);
    uint64_t aFirstIter = invGroup.getIterationNumber(aCallFirst.first, aCallFirst.second);
    uint64_t aLastIter = invGroup.getIterationNumber(aCallFirst.first, aCallLast.second);
    uint64_t bFirstIter = invGroup.getIterationNumber(bCallFirst.first, bCallFirst.second);
    uint64_t bLastIter = invGroup.getIterationNumber(bCallFirst.first, bCallLast.second);
    if(aFirstIter == bFirstIter && aLastIter == bLastIter){
      if(get<1>(alias) == 0 && get<2>(alias) > 1 && aFirstIter != aLastIter) // If multiple accesses to same addr, alias is inter iteration
        return false;
      else
        return true;
    }
    else
      return false;
  }
  else{
    /* Check first and last accesses to see if they occur in different iterations */
    uint64_t aFirstIter, aLastIter, bFirstIter, bLastIter;
    if(invGroup.containsInstruction(getInstrA())){
      aFirstIter = invGroup.getIterationNumber(getInstrA(), aFirst);
      aLastIter = invGroup.getIterationNumber(getInstrA(), aLast);
    }
    else{
      pair<uintptr_t, uint64_t> aCallFirst = callTraces.getRootCallIdAndInstance(getInstrA(), aFirst);
      pair<uintptr_t, uint64_t> aCallLast = callTraces.getRootCallIdAndInstance(getInstrA(), aLast);
      aFirstIter = invGroup.getIterationNumber(aCallFirst.first, aCallFirst.second);
      aLastIter = invGroup.getIterationNumber(aCallFirst.first, aCallLast.second);
    }
    if(invGroup.containsInstruction(getInstrB())){
      bFirstIter = invGroup.getIterationNumber(getInstrB(), bFirst);
      bLastIter = invGroup.getIterationNumber(getInstrB(), bLast);
    }
    else{
      pair<uintptr_t, uint64_t> bCallFirst = callTraces.getRootCallIdAndInstance(getInstrB(), bFirst);
      pair<uintptr_t, uint64_t> bCallLast = callTraces.getRootCallIdAndInstance(getInstrB(), bLast);
      bFirstIter = invGroup.getIterationNumber(bCallFirst.first, bCallFirst.second);
      bLastIter = invGroup.getIterationNumber(bCallFirst.first, bCallLast.second);
    }
    if(aFirstIter != bFirstIter || aLastIter != bLastIter){
      return false;
    }
    else if(get<2>(alias) == 1){
      return true;
    }
    else if(get<2>(alias) == 2){
      if(get<1>(alias) == 0 && aFirstIter == aLastIter)
        return false;
      else
        return true;
    }
    else
      return convertToBruteForceAlias().allAliasesAreIntraIteration(invGroup, invocNum, callTraces);
  }
}
#endif

#if 0
set<pair<uintptr_t, uintptr_t>> BruteForceAlias::getSetOfCallPairs(CallTraces& invCallTraces){
  set<pair<uintptr_t, uintptr_t>> callPairs;
  set<uintptr_t> subInstrs = invCallTraces.getAllSubInstructions();
  for(auto dep = deps.begin(); dep != deps.end(); dep++){
    uintptr_t instrA = getInstrA(), instrB = getInstrB();
    if(subInstrs.find(getInstrA()) != subInstrs.end())
      instrA = get<0>(invCallTraces.getRootCallIdAndInstance(getInstrA(), getSetA().getStart() + get<1>(*dep)));
    if(subInstrs.find(getInstrB()) != subInstrs.end())
      instrB = get<0>(invCallTraces.getRootCallIdAndInstance(getInstrB(), getSetB().getStart() + get<3>(*dep)));
    callPairs.insert(pair<uintptr_t, uintptr_t>(instrA, instrB));
  }
  return callPairs;
}

/* I can't think of any better way to do this right now, there may be some time saving tricks like if a sub instruction is
 * always under the same call then there's no need to do iterate over every dependence.
 */
set<pair<uintptr_t, uintptr_t>> PatternAlias::getSetOfCallPairs(CallTraces& invCallTraces){
  set<pair<uintptr_t, uintptr_t>> callPairs;
  set<uintptr_t> subInstrs = invCallTraces.getAllSubInstructions();
  if(get<1>(alias) > 0){
    for(uint64_t address = get<0>(alias); address < get<0>(alias) + get<1>(alias)*get<2>(alias); address += get<1>(alias)){
      uintptr_t instrA = getInstrA(), instrB = getInstrB();
      if(subInstrs.find(getInstrA()) != subInstrs.end())
        instrA = get<0>(invCallTraces.getRootCallIdAndInstance(getInstrA(), getSetA().getFirstDynamicInstanceFromAddress(address)));
      if(subInstrs.find(getInstrB()) != subInstrs.end())
        instrB = get<0>(invCallTraces.getRootCallIdAndInstance(getInstrB(), getSetB().getFirstDynamicInstanceFromAddress(address)));
      callPairs.insert(pair<uintptr_t, uintptr_t>(instrA, instrB));
    }
  }
  else{ //Multiple accesses to same address
    for(uint64_t instance = 0; instance < get<2>(alias); instance++){
      uintptr_t instrA = getInstrA(), instrB = getInstrB();
      if(subInstrs.find(getInstrA()) != subInstrs.end())
        instrA = get<0>(invCallTraces.getRootCallIdAndInstance(getInstrA(), getSetA().getFirstDynamicInstanceFromAddress(get<0>(alias)) + instance));
      if(subInstrs.find(getInstrB()) != subInstrs.end())
        instrB = get<0>(invCallTraces.getRootCallIdAndInstance(getInstrB(), getSetB().getFirstDynamicInstanceFromAddress(get<0>(alias)) + instance));
      callPairs.insert(pair<uintptr_t, uintptr_t>(instrA, instrB));
    }
  }
  return callPairs;
}
#endif

BruteForceAlias PatternAlias::convertToBruteForceAlias(){
  vector<tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>> deps;
  for(uint64_t i = 0; i < get<2>(alias); i++)
    deps.push_back(tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>(
      get<0>(alias) + i*get<1>(alias), //addressA
      get<1>(alias) > 0 ? getSetA().getFirstDynamicInstanceFromAddress(get<0>(alias) + i*get<1>(alias)) : i, //memSetInstanceA
      get<0>(alias) + i*get<1>(alias), //addressB
      get<1>(alias) > 0 ? getSetB().getFirstDynamicInstanceFromAddress(get<0>(alias) + i*get<1>(alias)) : i //memSetInstanceB
    ));
  return BruteForceAlias(getInstrA(), getSetA(), getInstrB(), getSetB(), deps);
}

ostream& operator<<(ostream& os, const AliasRec& a){
  return a.print(os);
}

ostream& operator<<(ostream& os, const BruteForceAlias& a){
  os << a.getInstrA() << ":";
  a.getSetA().printWithIterInfo(os);
  os << " "; 
  os << a.getInstrB() << ":";
  a.getSetB().printWithIterInfo(os);
  for(auto i = a.deps.begin(); i != a.deps.end(); i++)
    os << " (" << get<0>(*i) << "," << get<1>(*i) << "," << get<2>(*i) << "," << get<3>(*i) << ")";
  return os;
}

ostream& operator<<(ostream& os, const PatternAlias& a){
  os << a.getInstrA() << ":";
  a.getSetA().printWithIterInfo(os);
  os << " ";
  os << a.getInstrB() << ":";
  a.getSetB().printWithIterInfo(os);
  os << " (" << get<0>(a.getAlias()) << " " << get<1>(a.getAlias()) << " " << get<2>(a.getAlias()) << ")";
  return os;
}

void printClashes(vector<tuple<uintptr_t, MemSetEntry&, vector<tuple<uintptr_t, MemSetEntry&>>>> &clashes){
  typedef tuple<uintptr_t, MemSetEntry&> AccessRange;
  typedef tuple<uintptr_t, MemSetEntry&, vector<AccessRange>> AccessMultiMap;
  for(vector<AccessMultiMap>::iterator i = clashes.begin(); i != clashes.end(); i++){
    cout << "Instruction: " << get<0>(*i) << endl
         << "Write: " << get<1>(*i) << endl
         << "Clashes with: ";
    for(vector<AccessRange>::iterator j = get<2>(*i).begin(); j != get<2>(*i).end(); j++){
      cout << "( " << get<0>(*j) << " " << get<1>(*j) << " ) ";
    }
    cout << endl << endl;
  }
}

void printClashingPairs(vector<pair<uintptr_t, uintptr_t>> p){
  for (auto i = p.begin(); i != p.end(); i++)
    cout << i->first << " " << i->second << ",";
  cout << endl;
}

/* Special case of brute force alias analysis where one MemSetEntry is constant (stride == 0) and the other variable */
vector<tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>> doBruteForceAliasAnalysis_Constant_Variable(MemSetEntry &constant, MemSetEntry &variable, int order, 
    vector<tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>> &clashingPairs){
  uint64_t lowerExtent = constant.getBase();
  uint64_t upperExtent = constant.getUpperExtent();
  MemSetEntry normalised = variable.getNormalised();
  int64_t offset = (lowerExtent > normalised.getLowerExtent()) ? -1*((lowerExtent - normalised.getLowerExtent())%normalised.getStride()) : (normalised.getLowerExtent() - lowerExtent);
  uint64_t lowestPossiblyClashingAddress = lowerExtent + offset;
  uint64_t searchWidth = std::min(upperExtent, (uint64_t)normalised.getUpperExtent()) - lowestPossiblyClashingAddress;
  uint64_t numberOfPossibleClashes = searchWidth/normalised.getStride() + 1;

  if((lowestPossiblyClashingAddress + normalised.getLength() - 1) < lowerExtent){
    lowestPossiblyClashingAddress += normalised.getStride();
    numberOfPossibleClashes--;
  }

  uint64_t dynInstOfFirstClash = variable.getFirstDynamicInstanceFromAddress(lowestPossiblyClashingAddress) - variable.getStart();
  uint64_t address = lowestPossiblyClashingAddress;
  uint64_t instance = dynInstOfFirstClash;

  for(uint64_t i = 0; i < numberOfPossibleClashes; i++){
    for(uintptr_t j  = 0; j < constant.getNumInstances(); j++){
      if (order == 0)
        clashingPairs.push_back(tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>(constant.getBase(), j, address, instance));
      else
        clashingPairs.push_back(tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>(address, instance, constant.getBase(), j));
    }
    address += normalised.getStride();
    instance += (variable.getStride() >= 0) ? 1 : -1;
  }

  return clashingPairs;
}


vector<tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>> doBruteForceAliasAnalysis(MemSetEntry &a, MemSetEntry &b){
  vector<tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>> clashingPairs;
  /* Try to single out some common examples of overlapping MemSetEntries, if that doesn't work, take the brute force approach */
  if(a.getStride() == 0 && b.getStride() != 0){
    doBruteForceAliasAnalysis_Constant_Variable(a, b, 0, clashingPairs);
  }
  else if(a.getStride() != 0 && b.getStride() == 0){
    doBruteForceAliasAnalysis_Constant_Variable(b, a, 1, clashingPairs);
  }
  else if(a.getStride() == 0 && b.getStride() == 0){
    for(uintptr_t i = 0; i < a.getNumInstances(); i++)
      for(uintptr_t j = 0; j < b.getNumInstances(); j++)
        clashingPairs.push_back(tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>(a.getAccessLower(i), i, b.getAccessLower(j), j));
  }
  else{
    /* Brute force approach, check all possible combinations */
    for(uintptr_t j = 0; j < b.getNumInstances(); j++)
      for(uintptr_t i = 0; i < a.getNumInstances(); i++)
        if(a.getAccessLower(i) <= b.getAccessUpper(j) && b.getAccessLower(j) <= a.getAccessUpper(i))
          clashingPairs.push_back(tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>(a.getAccessLower(i), i, b.getAccessLower(j), j));
  }
  return clashingPairs;
}

void recordDependencePair(uintptr_t effectiveAddrA, uintptr_t effectiveAddrB, set<pair<uintptr_t, uintptr_t>> &dependence_pairs, 
    bool recordReverse){
    dependence_pairs.insert(pair<uintptr_t, uintptr_t>(effectiveAddrA, effectiveAddrB));
    if(recordReverse)
      dependence_pairs.insert(pair<uintptr_t, uintptr_t>(effectiveAddrB, effectiveAddrA));
}

void recordDependencePairs(uintptr_t instrA, uintptr_t instrB, vector<AliasRec*> &aliases, set<pair<uintptr_t, uintptr_t>> &dependence_pairs,
    bool recordReverse, set<uintptr_t> subCallInstrs){
  if(subCallInstrs.find(instrA) != subCallInstrs.end() || subCallInstrs.find(instrB) != subCallInstrs.end()){
    for(auto alias = aliases.begin(); alias != aliases.end(); alias++){
      dependence_pairs.insert(pair<uintptr_t, uintptr_t>((**alias).getSetA().getEffectiveInstrID(), (**alias).getSetB().getEffectiveInstrID()));
      if(recordReverse)
        dependence_pairs.insert(pair<uintptr_t, uintptr_t>((**alias).getSetB().getEffectiveInstrID(), (**alias).getSetA().getEffectiveInstrID()));
    }
  }
  else{
    dependence_pairs.insert(pair<uintptr_t, uintptr_t>(instrA, instrB));
    if(recordReverse)
      dependence_pairs.insert(pair<uintptr_t, uintptr_t>(instrB, instrA));
  }
}

void outputDependencePairs(set<pair<uintptr_t, uintptr_t>> &rw_dependence_pairs, set<pair<uintptr_t, uintptr_t>> &ww_dependence_pairs){
  ofstream outputFile;
  outputFile.open("dependence_pairs.txt");
  //outputFile << "LoopID: " << loopID << endl;
  //outputFile << "Write Read Pairs: \n";
  outputFile << rw_dependence_pairs.size() << endl;
  for(auto i = rw_dependence_pairs.begin(); i != rw_dependence_pairs.end(); i++)
    outputFile << i->first << " " <<  i->second << endl;
  //outputFile << "Write Write Pairs: \n";
  outputFile << endl << ww_dependence_pairs.size() << endl;
  for(auto i = ww_dependence_pairs.begin(); i != ww_dependence_pairs.end(); i++)
    outputFile << i->first << " " <<  i->second << endl;
  outputFile.close();
}

bool isAliasBruteForce(MemSetEntry& write, MemSetEntry& read){
  /* Brute force approach, check all possible combinations */
  for(uintptr_t j = 0; j < read.getNumInstances(); j++)
    for(uintptr_t i = 0; i < write.getNumInstances(); i++)
      if(write.getAccessLower(i) <= read.getAccessUpper(j) && read.getAccessLower(j) <= write.getAccessUpper(i))
        return true;
  return false;
}

bool isAlias(MemSetEntry& write, MemSetEntry& read){
  if(write.getStride() == 0 && read.getStride() == 0)
    return true;

  /* If accesses are different lengths or not aligned do brute force check */
  if (write.getLength() != read.getLength() || !write.isAligned() || !read.isAligned()){
    return isAliasBruteForce(write, read);
  }

  MemSetEntry normalWrite;
  MemSetEntry normalRead;
  if(write.getStride() == 0)
    normalWrite = MemSetEntry(write.getBase(), write.getLength(), write.getLength(), 0, 0);
  else  
    normalWrite = write.getNormalised();
  if(read.getStride() == 0)
    normalRead = MemSetEntry(read.getBase(), write.getLength(), read.getLength(), 0, 0);
  else  
    normalRead = read.getNormalised();

  AliasT alias = dynamic_gcd(normalWrite.getBase(), normalRead.getBase(), 
                             normalWrite.getLast(), normalRead.getLast(), 
                             normalWrite.getStride(), normalRead.getStride());
  if (get<2>(alias) > 0){
    return true;
  }
  else {
    return false;
  }
}

vector<AliasRec*> getAliasRecs(uintptr_t instrA, uintptr_t instrB, vector<pair<MemSetEntry&, MemSetEntry&>> clashes, bool bruteForceOnly){
  vector<AliasRec*> aliases;
  for(auto msePair = clashes.begin(); msePair != clashes.end(); msePair++){
    MemSetEntry &write = msePair->first;
    MemSetEntry &read = msePair->second;
    if(bruteForceOnly || 
        (write.getLength() != read.getLength() || !write.isAligned() || !read.isAligned())){
      auto bruteForceResults = doBruteForceAliasAnalysis(write, read);
      if(bruteForceResults.size() > 0){
        aliases.push_back(new BruteForceAlias(instrA, write, instrB, read, bruteForceResults));
      }
    }
    else{
      if(write.getStride() == 0)
        if(read.getStride() == 0)
          aliases.push_back(new PatternAlias(instrA, write, instrB, read, AliasT(write.getBase(), 0, min(read.getNumInstances(), write.getNumInstances()))));
        else
          aliases.push_back(new PatternAlias(instrA, write, instrB, read, AliasT(write.getBase(), 0, 1)));
      else if(read.getStride() == 0)
        aliases.push_back(new PatternAlias(instrA, write, instrB, read, AliasT(read.getBase(), 0, 1)));
      else{
        MemSetEntry normalWrite = write.getNormalised();
        MemSetEntry normalRead = read.getNormalised();
        AliasT alias = dynamic_gcd(normalWrite.getBase(), normalRead.getBase(), 
                                   normalWrite.getLast(), normalRead.getLast(), 
                                   normalWrite.getStride(), normalRead.getStride());
        if (get<2>(alias) > 0){
          aliases.push_back(new PatternAlias(instrA, write, instrB, read, alias));
        }
      }
    }
  }
  return aliases;
}

void recordOverlaps(vector<AccessMultiMap> &clashes, MemSetEntry &mentry, MemSetEntry::intervalTree *tree, uintptr_t id1, uintptr_t id2){
  MemSetEntry::intervalVector overlaps;
  tree->findOverlapping(mentry.getLowerExtent(), mentry.getUpperExtent(), overlaps);
  vector<AccessRange> accessClashes;
  for(MemSetEntry::intervalVector::iterator ivlIt = overlaps.begin(); ivlIt != overlaps.end(); ivlIt++)
    accessClashes.push_back(AccessRange(id2, *(ivlIt->value)));
  if(accessClashes.size() > 0)
    clashes.push_back(AccessMultiMap(id1, mentry, accessClashes));
}

vector<pair<MemSetEntry&, MemSetEntry&>> findClashes(uintptr_t instrA, uintptr_t instrB, MemoryTrace &memoryTraces, bool WAW){
  vector<pair<MemSetEntry&, MemSetEntry&>> clashes;
  MemSet &readsB = WAW ? memoryTraces[instrB].getWriteSet() : memoryTraces[instrB].getReadSet();
  MemSetEntry::intervalTree *writeTreeA = memoryTraces[instrA].getWriteTree();
  for(auto memIt = readsB.begin(); memIt != readsB.end(); memIt++){
    MemSetEntry::intervalVector overlaps;
    writeTreeA->findOverlapping(memIt->getLowerExtent(), memIt->getUpperExtent(), overlaps);
    for(auto ivlIt = overlaps.begin(); ivlIt != overlaps.end(); ivlIt++){
      if(ivlIt->value->getIterationNumber() != memIt->getIterationNumber())
        clashes.push_back(pair<MemSetEntry&, MemSetEntry&>(*(ivlIt->value), *memIt));
    }
  }
  return clashes;
}

void printInstrPairMenu(pair<vector<pair<uintptr_t, uintptr_t>>, vector<pair<uintptr_t, uintptr_t>>> &pairMenu){
  cout << "rw_menu\n";
  for(auto i = pairMenu.first.begin(); i != pairMenu.first.end(); i++){
    cout << "(" << i->first << "," << i->second << ")";
    if(next(i, 1) != pairMenu.first.end() && next(i, 1)->first != i->first)
      cout << endl;
  }
  cout << endl;
  cout << "ww_menu\n";
  for(auto i = pairMenu.second.begin(); i != pairMenu.second.end(); i++){
    cout << "(" << i->first << "," << i->second << ")";
    if(next(i, 1) != pairMenu.second.end() && next(i, 1)->first != i->first)
      cout << endl;
  }
  cout << endl;
}

bool instructionPairIsInInstructionTrace(set<uintptr_t> instrList, set<uintptr_t> subInstrList, uintptr_t a, uintptr_t b){
  if(instrList.find(a) == instrList.end() && subInstrList.find(a) == subInstrList.end())
    return false;
  else if(instrList.find(b) == instrList.end() && subInstrList.find(b) == subInstrList.end())
    return false;
  else 
    return true;
}

/* 
 * Build two vectors of pairs of instruction IDs: write-read pairs and write-write pairs
*/
pair<vector<pair<uintptr_t, uintptr_t>>, vector<pair<uintptr_t, uintptr_t>>> buildInstrPairMenu(
      set<uintptr_t> instrList
    , set<uintptr_t> subInstrList
    , pair<vector<uintptr_t>, vector<uintptr_t>> memtraceInstrIDs){
  vector<pair<uintptr_t, uintptr_t>> rw_menu, ww_menu;

  for(auto wi = memtraceInstrIDs.second.begin(); wi != memtraceInstrIDs.second.end(); wi++){
    for(auto ri = memtraceInstrIDs.first.begin(); ri != memtraceInstrIDs.first.end(); ri++){
      if(instructionPairIsInInstructionTrace(instrList, subInstrList, *wi, *ri))
        rw_menu.push_back(pair<uintptr_t, uintptr_t>(*wi, *ri));
    }
    for(auto wi2 = wi; wi2 != memtraceInstrIDs.second.end(); wi2++){
      if(instructionPairIsInInstructionTrace(instrList, subInstrList, *wi, *wi2))
        ww_menu.push_back(pair<uintptr_t, uintptr_t>(*wi, *wi2));
    }
  }
  return pair<vector<pair<uintptr_t, uintptr_t>>, vector<pair<uintptr_t, uintptr_t>>>(rw_menu, ww_menu);
}

/* 
 * Build an instruction pair menu (as above) containing only the pairs which exist in the static DDG.
 * If the static DDG is missing real dependences (e.g. due to a bug in the static analysis) these will not be found.
*/
pair<vector<pair<uintptr_t, uintptr_t>>, vector<pair<uintptr_t, uintptr_t>>> buildInstrPairMenuFromDDG(
      set<uintptr_t> instrList
    , set<uintptr_t> subInstrList
    , pair<vector<uintptr_t>, vector<uintptr_t>> memtraceInstrIDs
    , string ddgfile){
  vector<pair<uintptr_t, uintptr_t>> rw_menu, ww_menu;
  vector<tuple<uintptr_t, uintptr_t, unsigned int>> static_ddg = parse_ddg(ddgfile);

  /* First get the menu of all possible combinations, then remove any which are not in the static ddg */
  pair<vector<pair<uintptr_t, uintptr_t>>, vector<pair<uintptr_t, uintptr_t>>> exhaustiveMenu =
    buildInstrPairMenu(instrList, subInstrList, memtraceInstrIDs);

  /* Check RAW and WAR deps */
  for(auto rw_pair : exhaustiveMenu.first){
    /* Check both instructions are in the main loop body (sub-call instructions must be left in regardless) */
    if(instrList.find(rw_pair.first) != instrList.end() && instrList.find(rw_pair.second) != instrList.end()){
      for(auto dep : static_ddg){
        if(((get<2>(dep) & DEP_MRAW) && get<0>(dep) == rw_pair.second && get<1>(dep) == rw_pair.first) || 
           ((get<2>(dep) & DEP_MWAR) && get<1>(dep) == rw_pair.second && get<0>(dep) == rw_pair.first)){
          rw_menu.push_back(rw_pair);
          continue;
        }
      }
    }
    else 
      rw_menu.push_back(rw_pair);
  }

  /* Check WAW deps */
  for(auto ww_pair : exhaustiveMenu.second){
    /* Check both instructions are in the main loop body (sub-call instructions must be left in regardless) */
    if(instrList.find(ww_pair.first) != instrList.end() && instrList.find(ww_pair.second) != instrList.end()){
      for(auto dep : static_ddg){
        if((get<2>(dep) & DEP_MWAW) && get<0>(dep) == ww_pair.first && get<1>(dep) == ww_pair.second){
          ww_menu.push_back(ww_pair);
        }
      }
    }
    else 
      ww_menu.push_back(ww_pair);
  }

  return pair<vector<pair<uintptr_t, uintptr_t>>, vector<pair<uintptr_t, uintptr_t>>>(rw_menu, ww_menu);
}

void freeAliasMemory(vector<AliasRec*> &aliases){
  for(vector<AliasRec*>::iterator i = aliases.begin(); i != aliases.end(); i++){
    delete *i;
  }
}

vector<uintptr_t> instructionListForChecking(MemoryTrace& memoryTrace, set<uintptr_t> &instrList, set<uintptr_t> &callList, set<uintptr_t>& subInstrList,
    uintptr_t x, uintptr_t y){
  vector<uintptr_t> instrIDs;  
  set<uintptr_t> instrIDSet;

  if(callList.find(x) != callList.end())
    copy(subInstrList.begin(), subInstrList.end(), inserter(instrIDSet, instrIDSet.begin()));
  else
    instrIDSet.insert(x);
  if(callList.find(y) != callList.end())
    copy(subInstrList.begin(), subInstrList.end(), inserter(instrIDSet, instrIDSet.begin()));
  else
    instrIDSet.insert(y);

  copy(instrIDSet.begin(), instrIDSet.end(), back_inserter(instrIDs));
  
  return instrIDs;
}

/* 
 * Get list of all instructions which have 
*/
vector<uintptr_t> getInstructionIDs(MemoryTrace& memoryTrace, set<uintptr_t> &instrList, set<uintptr_t> &callList, set<uintptr_t>& subInstrList){
  /* Get a list of the instruction IDs we are interested in */
  vector<uintptr_t> instrIDs;  
  copy(instrList.begin(), instrList.end(), back_inserter(instrIDs));

  /* Remove any instructions not found in the the memory trace */
  for(vector<uintptr_t>::iterator i = instrIDs.begin(); i != instrIDs.end();){
    if(memoryTrace.find(*i) == memoryTrace.end()){
      if(callList.find(*i) != callList.end())
        i = instrIDs.erase(i);
      else{
        cout << "Instruction " << *i << " found in loop trace but not in memory trace or call trace" << endl;
        //abort();
        i = instrIDs.erase(i);
      }
    }
    else 
      i++;
  }

  /* Add instructions from the call trace */
  for(auto i = subInstrList.begin(); i != subInstrList.end(); i++){
    instrIDs.push_back(*i);
  }

  return instrIDs;
}

uint64_t getNumInstancesInInvocation(uintptr_t id, InvocationGroupCfc& invocGroup, CallTraceLoopInvocationGroup& invocCallTrace, set<uintptr_t> subInstrList){
  return subInstrList.find(id) == subInstrList.end() ? 
             invocGroup.getNumInstances(id)
           : invocCallTrace.getNumInstances(id);
}

void invocation_analysis_memeff(InvocationGroupCfc &invocGroup, uint64_t invocNum, MemoryTrace &invMemoryTrace, 
    map<pair<uintptr_t, bool>, MemoryTraceStreamer*>& memtraceStreamers, 
    vector<uintptr_t> memtraceInstrList,
    CallTraceLoopInvocationGroup &invocCallTrace, 
    vector<pair<uintptr_t, uintptr_t>> &pairMenu, set<uintptr_t> &instrList, set<uintptr_t> callList, set<uintptr_t>subInstrList,
    bool WAW, set<pair<uintptr_t, uintptr_t>> &dependence_pairs, map<string, unsigned int> &args){

  /* Cache the number of instances of each instruction */
  map<uintptr_t, uint64_t> instructionInstances;
  for(auto i = memtraceInstrList.begin(); i != memtraceInstrList.end(); i++){
    instructionInstances[*i] = getNumInstancesInInvocation(*i, invocGroup, invocCallTrace, subInstrList);
  }

  /* Analyse each instruction pair in turn */
  for(auto instrPair = pairMenu.begin(); instrPair != pairMenu.end();){

    if(instructionInstances[instrPair->first] == 0 && instructionInstances[instrPair->second] == 0){
      instrPair++;
      continue;
    }

    /* Get the memory trace chunk for this invocation and these instruction */
    PDEBUG("  Get memory trace chunk\n");
    //uint64_t instrAInstances = getNumInstancesInInvocation(instrPair->first, invocGroup, invocCallTrace, subInstrList);
    //uint64_t instrBInstances = getNumInstancesInInvocation(instrPair->second, invocGroup, invocCallTrace, subInstrList);
    //uint64_t instrAInstances = instructionInstances[instrPair->first];
    //uint64_t instrBInstances = instructionInstances[instrPair->second];
    if(!invMemoryTrace[instrPair->first].hasWriteSet()){
      MemSet slice = memtraceStreamers[pair<uintptr_t, bool>(instrPair->first, true)]->getNextChunk(instructionInstances[instrPair->first]);
      slice.initialiseSliceIterator();
      invMemoryTrace[instrPair->first].setWriteSet(slice.getInvocationSliceWithIterTags(instrPair->first, invocGroup, 0, invocCallTrace));
    }
    if(WAW){
      if(!invMemoryTrace[instrPair->second].hasWriteSet()){
        MemSet slice = memtraceStreamers[pair<uintptr_t, bool>(instrPair->second, true)]->getNextChunk(instructionInstances[instrPair->second]);
        slice.initialiseSliceIterator();
        invMemoryTrace[instrPair->second].setWriteSet(slice.getInvocationSliceWithIterTags(instrPair->second, invocGroup, 0, invocCallTrace));
      }
    }
    else{
      if(!invMemoryTrace[instrPair->second].hasReadSet()){
        MemSet slice = memtraceStreamers[pair<uintptr_t, bool>(instrPair->second, false)]->getNextChunk(instructionInstances[instrPair->second]);
        slice.initialiseSliceIterator();
        invMemoryTrace[instrPair->second].setReadSet(slice.getInvocationSliceWithIterTags(instrPair->second, invocGroup, 0, invocCallTrace));
      }
    }

#if 0
    /* Get memory trace slice for these instructions */
    PDEBUG("  Get memory trace slices\n");
    MemoryTrace invMemoryTrace2;
    if(!invMemoryTrace2.hasStaticInstRec(instrPair->first)){
      invMemoryTrace2[instrPair->first] = masterMemoryTrace.getInvocationSliceWithIterTags(invocGroup, 0, invocCallTrace, instrPair->first);
      //cout << instrPair->first << ": \n";
      //invMemoryTrace2[instrPair->first].printWithIterInfo();
    }
    if(!invMemoryTrace2.hasStaticInstRec(instrPair->second)){
      invMemoryTrace2[instrPair->second] = masterMemoryTrace.getInvocationSliceWithIterTags(invocGroup, 0, invocCallTrace, instrPair->second);
      //cout << instrPair->second << ": \n";
      //invMemoryTrace2[instrPair->second].printWithIterInfo();
    }
    cout << invMemoryTrace2 << endl;
    cout << endl << endl << endl;
#endif

    /* For each instruction, build interval trees for read and write sets */
    PDEBUG("  Build interval trees\n");
    if(!invMemoryTrace[instrPair->first].hasWriteTree())
      invMemoryTrace[instrPair->first].buildWriteTree();

    XanBitSet* iterationsWithAdjacentDependence = xanBitSet_new(1);

    /* For each overlap in the tree, check if there is an alias and record it */
    PDEBUG("  Do alias checking\n");
    //uint64_t totalOverlaps = 0, aliases = 0, sameIter = 0, diffIter = 0;
    MemSet &readsB = WAW ? invMemoryTrace[instrPair->second].getWriteSet() : invMemoryTrace[instrPair->second].getReadSet();
    MemSetEntry::intervalTree *writeTreeA = invMemoryTrace[instrPair->first].getWriteTree();
    bool pairComplete = false;
    for(auto memIt = readsB.begin(); memIt != readsB.end(); memIt++){
      for(auto oiter = writeTreeA->overlapIteratorBegin(memIt->getLowerExtent(), memIt->getUpperExtent());
               oiter != writeTreeA->overlapIteratorEnd();
               oiter = writeTreeA->overlapIteratorNext(memIt->getLowerExtent(), memIt->getUpperExtent(), oiter)){
        auto ivlIt = oiter.second;
        //totalOverlaps++;
        if(ivlIt->value->getIterationNumber() != memIt->getIterationNumber()){
          /* Found an overlap, now check if it is an alias */
          if(isAlias(*(ivlIt->value), *memIt)){
            //aliases++;
            if(args["adjacent"]){
              int64_t it1 = ivlIt->value->getIterationNumber();
              int64_t it2 = memIt->getIterationNumber();
              if(abs(it1 - it2) < 2){
                xanBitSet_setBit(iterationsWithAdjacentDependence, ivlIt->value->getIterationNumber());
                xanBitSet_setBit(iterationsWithAdjacentDependence, memIt->getIterationNumber());
              }
            }
            else{
              /* Record dependence pair */
              recordDependencePair(ivlIt->value->getEffectiveInstrID(), memIt->getEffectiveInstrID(), dependence_pairs, WAW);
              /* If neither instruction was inside a call, remove pair from menu so it is not checked again */
              if((subInstrList.find(instrPair->first) == subInstrList.end() || invocCallTrace.isConstantCallID(instrPair->first)) && 
                 (subInstrList.find(instrPair->second) == subInstrList.end() || invocCallTrace.isConstantCallID(instrPair->second))){
                instrPair = pairMenu.erase(instrPair);
                pairComplete = true;
                break;
              }
            }
          }
          //else
          //  diffIter++;
        }
        //else
        //  sameIter++;
      }
      if(pairComplete)
        break;
    }
    //cout << "totalOverlaps: " << totalOverlaps << ", aliases: " << aliases << ", sameIter: " << sameIter << ", diffIter: " << diffIter << endl;

    if(args["adjacent"]){
      if(xanBitSet_getCountOfBitsSet(iterationsWithAdjacentDependence) != xanBitSet_length(iterationsWithAdjacentDependence)){
        //cout << "Erasing pair " << instrPair->first << ", " << instrPair->second << endl;
        //cout << xanBitSet_getCountOfBitsSet(iterationsWithAdjacentDependence) << ", " << xanBitSet_length(iterationsWithAdjacentDependence) << endl;
        instrPair = pairMenu.erase(instrPair);
        pairComplete = true;
      }
      //else
      //  cout << "Keeping pair " << instrPair->first << ", " << instrPair->second << endl;
    }
    xanBitSet_free(iterationsWithAdjacentDependence);

    if(!pairComplete)
      instrPair++;

#if 0

    /* Find all address range clashes (rw_clashes, ww_clashes) */
    PDEBUG("  Find clashes\n");
    vector<pair<MemSetEntry&, MemSetEntry&>> clashes = findClashes(instrPair->first, instrPair->second, invMemoryTrace, WAW); 
    if(clashes.empty()){
      instrPair++;
      continue;
    }
    //for(auto c = clashes.begin(); c != clashes.end(); c++) {
    //  cout << instrPair->first << ": "; c->first.printWithIterInfo(); 
    //  cout << instrPair->second << ": "; c->second.printWithIterInfo(); 
    //  cout << endl;
    //}
    //printClashes(clashes.first);

    /* Find alias patterns in overlapping address ranges */
    PDEBUG("  Get alias recs\n");
    vector<AliasRec*> aliases = getAliasRecs(instrPair->first, instrPair->second, clashes, args["bruteforce"]);
    clashes.clear();
    //for(auto a = rw_aliases.begin(); a != rw_aliases.end(); a++){
    //  cout << **a << endl;
    //  cout << (**a).getSetA() << " " << (**a).getSetB() << endl;
    //}
    //if(rw_aliases.size() > 0) exit(0);

    //for(auto invocGroup = rw_aliases.begin(); invocGroup != rw_aliases.end(); invocGroup++)
    //  cout << **invocGroup << endl;
    //for(auto invocGroup = ww_aliases.begin(); invocGroup != ww_aliases.end(); invocGroup++)
    //  cout << **invocGroup << endl;

    /* If we are running the check mode, output aliases related to the check instructions */
    if(args["check"] != 0){
      for(auto a = aliases.begin(); a != aliases.end(); a++){
        if(((**a).getSetA().getEffectiveInstrID() == args["check"] && (**a).getSetB().getEffectiveInstrID() == args["checkarg"]) ||
           ((**a).getSetA().getEffectiveInstrID() == args["checkarg"] && (**a).getSetB().getEffectiveInstrID() == args["check"]))
          cout << **a << endl;
      }
    }

    if(aliases.size() > 0){
      /* Add any new dependence pairs to the sets */
      PDEBUG("  Add new dependence pairs\n");
      recordDependencePairs(instrPair->first, instrPair->second, aliases, dependence_pairs, WAW, subInstrList);

      /* Remove these instructions from the menu so they will not be checked again */
      if(subInstrList.find(instrPair->first) == subInstrList.end() && subInstrList.find(instrPair->second) == subInstrList.end())
        instrPair = pairMenu.erase(instrPair);
      else
        instrPair++;
    }
    else 
      instrPair++;

    
    /* Cleanup */
    freeAliasMemory(aliases);
#endif
  }
}

void endOfInvocationCleanup(StreamParseLoopRec& sloop, list<LoopTraceEntry>::iterator sloopIterator, InvocationGroupCfc *i, uint64_t invocNum){
  if(invocNum + 1 > sloopIterator->finalInvocationNumber())
    sloopIterator = sloop.erase(sloopIterator);
}

void loop_analysis_memeff(map<string, unsigned int> &args, StreamParseCallTrace& callTraces, 
    set<uintptr_t> instrList, set<uintptr_t> callList, set<uintptr_t> subInstrList, uint64_t numInvocations){
 
  /* Allocate sets to store dependence pairs */
  set<pair<uintptr_t, uintptr_t>> rw_dependence_pairs;
  set<pair<uintptr_t, uintptr_t>> ww_dependence_pairs;

  /* Get list of instructions in the memory trace */
  pair<vector<uintptr_t>, vector<uintptr_t>> memtraceInstrIDs = findMemoryTraces();
  vector<uintptr_t> memtraceInstrList;
  copy(memtraceInstrIDs.first.begin(), memtraceInstrIDs.first.end(), back_inserter(memtraceInstrList));
  copy(memtraceInstrIDs.second.begin(), memtraceInstrIDs.second.end(), back_inserter(memtraceInstrList));

  /* 
   * Get a list of the instruction IDs we are interested in.
   * If options -x -y have been set, only get a menu of these instructions
  */
  //vector<uintptr_t> instrIDs;
  //if(args["check"] != 0)
  //  instrIDs = instructionListForChecking(memoryTrace, instrList, callList, subInstrList, args["check"], args["checkarg"]);
  //else
  //  instrIDs = getInstructionIDs(memoryTrace, instrList, callList, subInstrList);
  
  /* Get menu of instruction pairs which we are interested in */
  pair<vector<pair<uintptr_t, uintptr_t>>, vector<pair<uintptr_t, uintptr_t>>> pairMenu;
  if(args["checkstaticdepsonly"])
    pairMenu = buildInstrPairMenuFromDDG(instrList, subInstrList, memtraceInstrIDs, "static_inst_dependences.txt");
  else if(args["adjacent"])
    pairMenu = buildInstrPairMenuFromDDG(instrList, subInstrList, memtraceInstrIDs, "dynamic_ddg.txt");
  else
    pairMenu = buildInstrPairMenu(instrList, subInstrList, memtraceInstrIDs);
  //printInstrPairMenu(pairMenu);

  /* Empty call trace for when there is no call trace */
  CallTraceLoopInvocationGroup emptyCallTrace;

  /* Sparse MemSetEntries with large strides are likely to cause false clashes, split these entries */
  //memoryTrace.splitLargeStrides();

  /* Create a map to save a running count of instructions processed from the call trace, 
   * this makes it easier to build the call trace cache later */
  map<uintptr_t, uint64_t> subInstrRunningCount;
  for(auto i = subInstrList.begin(); i != subInstrList.end(); i++)
    subInstrRunningCount[*i] = 0;

  callTraces.buildLuts();

  ///* Initialise iterators for slicing out invocations from the memory trace */
  //memoryTrace.initialiseSliceIterators();

  /* Create the memory trace streamers */
  map<pair<uintptr_t, bool>, MemoryTraceStreamer*> memtraceStreamers;
  for(auto i = memtraceInstrIDs.first.begin(); i != memtraceInstrIDs.first.end(); i++)
    memtraceStreamers[pair<uintptr_t, bool>(*i, false)] = new MemoryTraceStreamer(*i, "memory_accesses/memory_accesses." + to_string(*i) + ".r.txt.bz2",false);
  for(auto i = memtraceInstrIDs.second.begin(); i != memtraceInstrIDs.second.end(); i++)
    memtraceStreamers[pair<uintptr_t, bool>(*i, true)] = new MemoryTraceStreamer(*i, "memory_accesses/memory_accesses." + to_string(*i) + ".w.txt.bz2",true);

  /* Create a map of dynamic instruction counts (count how many instances we've accounted for so far */
  map<uintptr_t, uint64_t> instrCounts;
  for(auto i = instrList.begin(); i != instrList.end(); i++)
    instrCounts[*i] = 0;

  /* Create a TimeoutCounter to record percentage of analysis completed at timeout */
  TimeoutCounter timeoutCounter;


#if 0
  // TODO Move this stuff to a separate function (probably should be part of StreamParseLoopRec)
  PDEBUG("Getting first invocation group\n");
  uint64_t invocNum = 0;
  StreamParseLoopRec sloop;
  list<LoopTraceEntry>::iterator sloopIterator;
  int remaining = 1;
  LoopTraceEntry l;
  //cout << "Get first loop entry\n";
  remaining = loopTraceGetNextEntry(l);
  sloop.push_back(l);
  //cout << "Compute data structures\n";
  sloop.back().precomputeDataStructures();
  while(sloop.size() > 0 || remaining == 1){

    /* Check for timeouts */
    timeoutCounter.checkTime();
    if(timeoutCounter.isTimedOut()){
      timeoutCounter.dumpStats("cam_timeout_stats.csv", invocNum, numInvocations);
      exit(0);
    }

    PDEBUG("Get next invocation group\n");
    vector<pair<uint64_t, uint64_t>>::iterator match;
    InvocationGroupCfc* i = NULL;
    /* Search through the invocation groups that have already been parsed to find a match */
    list<LoopTraceEntry>::iterator lteIter = sloop.getLoopTraceEntryFromInvocationNumber(invocNum);
    if(lteIter != sloop.end()){
      i = lteIter->getInvocationGroupPointer();
      sloopIterator = lteIter;
    }
    /* If no match was found, parse new groups until a match is found */
    bool finished = false;
    while(i == NULL){
      LoopTraceEntry entry;
      remaining = loopTraceGetNextEntry(entry);
      if(remaining == 0){
        finished = true;
        break;
      }
      else{
        sloop.push_back(entry);
      }
      sloop.back().precomputeDataStructures();
      match = lower_bound(sloop.back().begin(), sloop.back().end(), invocNum, LoopTraceEntry::compareRange);
      if(match != sloop.back().end() && invocNum >= match->first){
        i = sloop.back().getInvocationGroupPointer();
        sloopIterator = sloop.end();
        sloopIterator--;
      }
    }
    
    /* No more invocations */
    if(finished)
      break;
#endif

  StreamParseLoopRec sloop;
  uint64_t invocNum = -1;
  for(auto invi = sloop.ii_begin(); invi != sloop.ii_end(); invi = sloop.ii_next(invi)){
    invocNum = invi.second;
    InvocationGroupCfc& invGroup = *invi.first->getInvocationGroupPointer();

    /* Check for timeouts */
    timeoutCounter.checkTime();
    if(timeoutCounter.isTimedOut()){
      timeoutCounter.dumpStats("cam_timeout_stats.csv", invocNum, numInvocations);
      exit(0);
    }

    /* Verbose output */
    if(args["verbosity"] >= 1){ 
      if(args["verbosity"] >= 2)
        cout << "CAM: Invocation " << invocNum << " of " << numInvocations << endl;
      else if(invocNum%(((numInvocations-1)/100)+1) == 0)
        cout << "CAM: Invocation " << invocNum << " of " << numInvocations << endl;
    }

    if(invGroup.isEmpty()){
      //endOfInvocationCleanup(sloop, sloopIterator, i, invocNum);
      //invocNum++;
      continue;
    }

    //i->printNumInstancesLut();

    /* Get the slice of the call trace relating to this invocation */
    PDEBUG("Get call trace slice\n");
    CallTraceLoopInvocationGroup* invocCallTrace;
    if(!callTraces.empty())
      invocCallTrace = &callTraces.getInvocCallTraceFromInvocNumber(invocNum);
    else
      invocCallTrace = &emptyCallTrace;

    /* Build cache to make looking up callId from from instrId more efficient */
    if(!invocCallTrace->isCallTraceCacheBuilt())
      invocCallTrace->buildCallTraceCache(invGroup, subInstrRunningCount, callList);

    /* Create a memory trace to store this invocation's memory accesses */
    MemoryTrace invMemoryTrace;

    /* Do RAW/WAR analysis and WAW analysis in turn */
    invocation_analysis_memeff(invGroup, invocNum, invMemoryTrace, memtraceStreamers, memtraceInstrList, *invocCallTrace, pairMenu.first, instrList, callList, subInstrList, false, rw_dependence_pairs, args);
    invocation_analysis_memeff(invGroup, invocNum, invMemoryTrace, memtraceStreamers, memtraceInstrList, *invocCallTrace, pairMenu.second, instrList, callList, subInstrList, true, ww_dependence_pairs, args);

    /* Update the instruction counts */
    for(auto instr = instrList.begin(); instr != instrList.end(); instr++){
      instrCounts[*instr] += invGroup.getNumInstances(*instr);
    }

    //endOfInvocationCleanup(sloop, sloopIterator, i, invocNum);

    //invocNum++;
  }

  assert(invocNum + 1 == numInvocations);

  /* Release the memory trace streamers */
  for(auto s = memtraceStreamers.begin(); s != memtraceStreamers.end(); s++)
    delete s->second;

  /* Finally dump the dependence pairs to a file */
  if(args["adjacent"]){
    for (auto pair : pairMenu.first){
      cout << pair.first << " " << pair.second << endl;
    }
    for (auto pair : pairMenu.second){
      cout << pair.first << " " << pair.second << endl;
    }
  }
  else{
    outputDependencePairs(rw_dependence_pairs, ww_dependence_pairs);
  }

}

void dependence_analysis(map<string, unsigned int> &args, StreamParseCallTrace &callTraces, 
    set<uintptr_t> instrList, uint64_t numInvocations, set<uintptr_t> callList, set<uintptr_t> subInstrList){

  loop_analysis_memeff(args, callTraces, instrList, callList, subInstrList, numInvocations);
}
