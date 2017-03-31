
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

#include "StaticLoopRec.h"
#include "parser_wrappers.h"
#include <iostream>
#include <assert.h>
#include <algorithm>

/*
 * IterationGroupCfc
*/

IterationGroupCfc::IterationGroupCfc() {}

IterationGroupCfc::IterationGroupCfc(uint64_t n, vector<CompressionPattern>& cf){
  repetitions.addPair(n, n);
  controlFlow = cf;
}

bool IterationGroupCfc::isEmpty() const {
  return (controlFlow.empty() && numInstancesLut.empty());
}

void IterationGroupCfc::addIterationPattern(uint64_t startIteration, uint64_t endIteration){
  repetitions.addPair(startIteration, endIteration);
}

void IterationGroupCfc::addIterationPattern(uint64_t iteration){
  repetitions.addPair(iteration, iteration);
}

void IterationGroupCfc::addIntoControlFlow(CompressionPattern& cp){
  controlFlow.push_back(cp);
}

uint64_t IterationGroupCfc::getFirstIteration(){
  return repetitions.getFirstInstance();
}

uint64_t IterationGroupCfc::getLastIteration(){
  return repetitions.getLastInstance();
}

bool IterationGroupCfc::isSingleIteration(){
  return repetitions.isSingleton();
}

IterationGroupCfc IterationGroupCfc::extractFirstIteration(){
  uint64_t firstIteration = getFirstIteration();
  repetitions.removeFirstInstance();
  return IterationGroupCfc(firstIteration, controlFlow);
}

IterationGroupCfc IterationGroupCfc::extractLastIteration(){
  uint64_t lastIteration = getLastIteration();
  repetitions.removeLastInstance();
  return IterationGroupCfc(lastIteration, controlFlow);
}

void IterationGroupCfc::buildNumberOfInstancesLut(){
  numInstancesLut.clear();
  for(auto cp = controlFlow.begin(); cp != controlFlow.end(); cp++){
    map<uintptr_t, uint64_t> cpInstances = cp->getNumberOfInstancesMap();
    for(auto i = cpInstances.begin(); i != cpInstances.end(); i++){
      numInstancesLut[i->first] += i->second;
    }
  }

  /* Discard controlflow, this discards information but reduces memory consumption */
  controlFlow.clear();
}

uint64_t IterationGroupCfc::getNumberOfInstancesInSingleIteration(uintptr_t instrID){
  return numInstancesLut[instrID];
}

RepetitionPattern& IterationGroupCfc::getRepetitionPattern(){
  return repetitions;
}

vector<CompressionPattern>& IterationGroupCfc::getControlFlow(){
  return controlFlow;
}

uint64_t IterationGroupCfc::getNumberOfIterations_slow(){
  uint64_t n = 0;
  for(auto i = repetitions.begin(); i != repetitions.end(); i++)
    n += i->second - i->first + 1;
  return n;
}

const map<uintptr_t, uint64_t>& IterationGroupCfc::getNumInstancesLut(){
  return numInstancesLut;
}

ostream& operator<<(ostream& os, const IterationGroupCfc& iterGroup){
  os << iterGroup.repetitions;
  for(auto i = iterGroup.controlFlow.begin(); i != iterGroup.controlFlow.end(); i++)
    os << *i;
  os << endl;
  return os;
}

/*
 * InvocationGroupCfc
*/

bool InvocationGroupCfc::isEmpty() const {
  if(iterations.size() == 0){
    return true;
  }
  else{
    for(auto i = iterations.begin(); i != iterations.end(); i++)
      if(!i->isEmpty())
        return false;
    return true;
  }
}

uint64_t InvocationGroupCfc::numIterationGroups() const {
  return iterations.size();
}

uint64_t InvocationGroupCfc::getNumInstances(uintptr_t instrID){
  return invocationInstancesLut[instrID];
}

uint64_t InvocationGroupCfc::getNumberOfIterations(){
  return iterLut.getLastInstance() + 1;
}

IterationGroupCfc &InvocationGroupCfc::addIterationGroup(){
  iterations.push_back(IterationGroupCfc());
  return iterations.back();
}

void InvocationGroupCfc::addIterationPattern(uint64_t startIteration, uint64_t endIteration){
  iterations.back().addIterationPattern(startIteration, endIteration);
}

void InvocationGroupCfc::addIterationPattern(uint64_t iteration){
  iterations.back().addIterationPattern(iteration);
}

void InvocationGroupCfc::addCfc(CompressionPattern &cp){
  iterations.back().addIntoControlFlow(cp);
}

RepetitionPatternLookup<IterationGroupCfc>::iterator InvocationGroupCfc::iterationIteratorBegin(){
  return iterLut.begin();
}

RepetitionPatternLookup<IterationGroupCfc>::iterator InvocationGroupCfc::iterationIteratorNext(RepetitionPatternLookup<IterationGroupCfc>::iterator ii){
  return iterLut.next(ii);
}

RepetitionPatternLookup<IterationGroupCfc>::iterator InvocationGroupCfc::iterationIteratorEnd(){
  return iterLut.end();
}

IterationGroupCfc* InvocationGroupCfc::getIterationFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii){
  return iterLut.getDataFromII(ii);
}

uint64_t InvocationGroupCfc::getNumIterationsFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii){
  return iterLut.getEndFromII(ii) - iterLut.getStartFromII(ii) + 1;
}

uint64_t InvocationGroupCfc::getStartIterationFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii){
  return iterLut.getStartFromII(ii);
}

uint64_t InvocationGroupCfc::getNumInstancesFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii, uintptr_t instrID){
  return iterLut.getDataFromII(ii)->getNumberOfInstancesInSingleIteration(instrID);
}

uint64_t InvocationGroupCfc::getIterationNumberFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii, uintptr_t instrID){
  return iterLut.getRangeNumFromII(ii);
}

IterationGroupCfc* InvocationGroupCfc::getIterationGroupFromIterationNumber(uint64_t n){
  return iterLut.lookupData(n);
}

void InvocationGroupCfc::mergeIterationInfo(InvocationGroupCfc& inv){
  auto compareLast = [] (IterationGroupCfc& a, IterationGroupCfc& b) { 
    return a.getLastIteration() < b.getLastIteration();
  };
  auto compareFirst = [] (IterationGroupCfc& a, IterationGroupCfc& b) { 
    return a.getFirstIteration() < b.getFirstIteration();
  };
  vector<IterationGroupCfc>::iterator maxExisting = max_element(iterations.begin(), iterations.end(), compareLast);
  vector<IterationGroupCfc>::iterator minNew = min_element(inv.iterations.begin(), inv.iterations.end(), compareFirst);
  bool copiedMinNew = false;
  if(maxExisting != iterations.end() && maxExisting->getLastIteration() == minNew->getFirstIteration()){
    IterationGroupCfc* workingOn;
    if(!maxExisting->isSingleIteration()){
      iterations.push_back(maxExisting->extractLastIteration());
      workingOn = &iterations.back();
    }
    else
      workingOn = &*maxExisting;
    /* Add in the overlapping iterations's control flow */
    for(auto i = minNew->getControlFlow().begin(); i != minNew->getControlFlow().end(); i++)
      workingOn->addIntoControlFlow(*i);
    copiedMinNew = true;
  }

  /* Add all remaining instances into the original */
  for(auto i = inv.iterations.begin(); i != inv.iterations.end(); i++){
    if(copiedMinNew && i == minNew){
      if(!i->isSingleIteration()){
        i->extractFirstIteration();
        iterations.push_back(*i);
      }
    }
    else{
      iterations.push_back(*i);
    }
  }
}

void InvocationGroupCfc::buildIterationNumInstancesLuts(){
  for(auto iterGroup = iterations.begin(); iterGroup != iterations.end(); iterGroup++)
    iterGroup->buildNumberOfInstancesLut();
}

void InvocationGroupCfc::buildIterGroupLut(){
  iterLut.clearLut();
  vector<pair<RepetitionPattern&, IterationGroupCfc*>> lutInput;
  for(auto i = iterations.begin(); i != iterations.end(); i++){
    lutInput.push_back(pair<RepetitionPattern&, IterationGroupCfc*>(i->getRepetitionPattern(), &*i));
  }
  iterLut.buildLut(lutInput);
}

void InvocationGroupCfc::buildInvocationNumInstancesLut(){
  invocationInstancesLut.clear();
  for(auto i = iterations.begin(); i != iterations.end(); i++){
    auto numInstances = i->getNumInstancesLut();
    auto numIterations = i->getNumberOfIterations_slow();
    for(auto entry = numInstances.begin(); entry != numInstances.end(); entry++){
      invocationInstancesLut[entry->first] += numIterations*(entry->second);
    }
  }
}

void InvocationGroupCfc::buildInstructionInstanceLut(){
  for(auto ii = iterLut.begin(); ii != iterLut.end(); ii = iterLut.nextEntry(ii)){
    /* Map of instruction ID -> number of instances for this iteration group */
    const map<uintptr_t, uint64_t>& iterationInstances = getIterationFromII(ii)->getNumInstancesLut();
    for(auto i = iterationInstances.begin(); i != iterationInstances.end(); i++){
      vector<InstructionInstanceLutEntry>& lut = instructionInstanceLut[i->first];
      if(lut.empty()){
        lut.push_back(InstructionInstanceLutEntry(0, i->second*(getNumIterationsFromII(ii)) - 1, getStartIterationFromII(ii), i->second));
      }
      /* If this entry has the same instances per iter as the previous lut entry and there are no 
       * intervening iterations with no instances for this instruction, just update the last lut entry */
      else if(get<3>(lut.back()) == i->second && 
          getStartIterationFromII(ii) == (get<2>(lut.back()) + ((get<1>(lut.back()) - get<0>(lut.back()) + 1)/get<3>(lut.back())))){
        get<1>(lut.back()) += i->second*(getNumIterationsFromII(ii));
      }
      else{
        InstructionInstanceLutEntry& prev = lut.back();
        lut.push_back(InstructionInstanceLutEntry(
          get<1>(prev) + 1, 
          get<1>(prev) + i->second*(getNumIterationsFromII(ii)), 
          getStartIterationFromII(ii),
          i->second));
      }
    }
  }
}

void InvocationGroupCfc::precomputeDataStructures(){
  buildIterationNumInstancesLuts();
  buildIterGroupLut();
  buildInvocationNumInstancesLut();
  buildInstructionInstanceLut();
}

void InvocationGroupCfc::printNumInstancesLut(){
  for(auto i = invocationInstancesLut.begin(); i != invocationInstancesLut.end(); i++)
    cout << i->first << ": " << i->second << endl;
}

void InvocationGroupCfc::printInstructionInstancesLut(){
  for(auto i : instructionInstanceLut){
    cout << i.first << endl;
    for(auto j : i.second){
      cout << "  " << get<0>(j) << "->" << get<1>(j) << ": " << get<2>(j) << "," << get<3>(j) << endl;
    }
  }
}

void InvocationGroupCfc::printIterLut(){
  cout << iterLut << endl;
}

bool InvocationGroupCfc::containsInstruction(uintptr_t instrID){
  return invocationInstancesLut[instrID] > 0;
}

uint64_t InvocationGroupCfc::getIterationNumber(uintptr_t instrID, uint64_t instance){
  auto compare = [] (const InstructionInstanceLutEntry& entry, uint64_t val) { return get<1>(entry) < val; };
  InstructionInstanceLutEntry& lutEntry = *lower_bound(instructionInstanceLut[instrID].begin(), instructionInstanceLut[instrID].end(), instance, compare);
  uint64_t offsetWithinLutEntry = instance - get<0>(lutEntry);
  return get<2>(lutEntry) + offsetWithinLutEntry/get<3>(lutEntry);
}

void InvocationGroupCfc::clear(){
  iterations.clear();
  invocationInstancesLut.clear();
  instructionInstanceLut.clear();
  iterLut.clearLut();
}

ostream& operator<<(ostream& os, const InvocationGroupCfc& invGroup){
  for(auto i = invGroup.iterations.begin(); i != invGroup.iterations.end(); i++){
    os << *i;
  }
  return os;
}

/* 
 * LoopTraceEntry 
*/

LoopTraceEntry::LoopTraceEntry() {}

LoopTraceEntry::LoopTraceEntry(uint64_t n, InvocationGroupCfc& inv){
  invocationRepetitionPattern.addPair(n, n);
  invocation = inv;
}

bool LoopTraceEntry::compareRange(const pair<uint64_t, uint64_t> & left, uint64_t val){
  return left.second < val;
}

uint64_t LoopTraceEntry::finalInvocationNumber(){
  return invocationRepetitionPattern.getLastInstance();
}

void LoopTraceEntry::addCfc(CompressionPattern& cp){
  invocation.addCfc(cp);
}

void LoopTraceEntry::addInvocationPattern(uint64_t start, uint64_t end){
  invocationRepetitionPattern.addPair(start, end);
}
void LoopTraceEntry::addInvocationPattern(uint64_t invoc){
  addInvocationPattern(invoc, invoc);
}

void LoopTraceEntry::addIterationPattern(uint64_t start, uint64_t end){
  invocation.addIterationPattern(start, end);
}
void LoopTraceEntry::addIterationPattern(uint64_t iter){
  invocation.addIterationPattern(iter, iter);
}
void LoopTraceEntry::addIterationGroup(){
  invocation.addIterationGroup();
}

const RepetitionPattern& LoopTraceEntry::getRepetitionPattern() const {
  return invocationRepetitionPattern;
}

RepetitionPattern::iterator LoopTraceEntry::begin(){
  return invocationRepetitionPattern.begin();
}
RepetitionPattern::iterator LoopTraceEntry::end(){
  return invocationRepetitionPattern.end();
}

RepetitionPattern::const_iterator LoopTraceEntry::begin() const {
  return invocationRepetitionPattern.begin();
}
RepetitionPattern::const_iterator LoopTraceEntry::end() const {
  return invocationRepetitionPattern.end();
}

void LoopTraceEntry::clear(){
  invocation.clear();
  invocationRepetitionPattern.clear();
}

bool LoopTraceEntry::isEmpty(){
  return invocationRepetitionPattern.empty();
}

bool LoopTraceEntry::lastAndFirstInvocationOverlap(LoopTraceEntry& other){
  return getLastInvocationNumber() == other.getFirstInvocationNumber();
}

uint64_t LoopTraceEntry::getFirstInvocationNumber(){
  return invocationRepetitionPattern.getFirstInstance();
}

uint64_t LoopTraceEntry::getLastInvocationNumber(){
  return invocationRepetitionPattern.getLastInstance();
}

bool LoopTraceEntry::isASingleInvocation(){
  return getLastInvocationNumber() == getFirstInvocationNumber();
}

LoopTraceEntry LoopTraceEntry::extractFirstInvocationIntoNewGroup(){
  LoopTraceEntry newEntry(getFirstInvocationNumber(), invocation);
  invocationRepetitionPattern.deleteFirstInstance();
  return newEntry;
}

LoopTraceEntry LoopTraceEntry::extractLastInvocationIntoNewGroup(){
  LoopTraceEntry newEntry(getLastInvocationNumber(), invocation);
  invocationRepetitionPattern.deleteLastInstance();
  return newEntry;
}

vector<LoopTraceEntry> LoopTraceEntry::mergeIntoAndReturnRemaining(LoopTraceEntry& other){
  vector<LoopTraceEntry> newGroups;

  /* If there is no overlap, no changes need be made */
  if(other.isEmpty() || !lastAndFirstInvocationOverlap(other)){
    if(!other.isEmpty())
      newGroups.push_back(other);
    return newGroups;
  }

  assert(getLastInvocationNumber() == other.getFirstInvocationNumber());

  LoopTraceEntry* workingOn = this;

  /* If this is group has multiple invocations, split off the previous ones and add a new group with just this invocation */
  if(!isASingleInvocation()){
    newGroups.push_back(extractLastInvocationIntoNewGroup());
    workingOn = &newGroups.back();
  }

  workingOn->invocation.mergeIterationInfo(other.invocation);

  /* If the other group had more than one invocation, the merged invocation should be removed from that group */
  if(!other.isASingleInvocation()){
    other.extractFirstInvocationIntoNewGroup();
    newGroups.push_back(other);
  }

  return newGroups;
}

void LoopTraceEntry::precomputeDataStructures(){
  invocation.precomputeDataStructures();
}

InvocationGroupCfc* LoopTraceEntry::getInvocationGroupPointer(){
  return &invocation;
}

uint64_t LoopTraceEntry::getNumberOfInvocations_slow(){
  uint64_t n = 0;
  for(auto i : invocationRepetitionPattern)
    n += i.second - i.first + 1;
  return n;
}

/* 
 * StreamParseLoopRec
*/ 

StreamParseLoopRec::StreamParseLoopRec() : loopTraceEntriesSize(0), streamer(LoopTraceStreamer()) {}

list<LoopTraceEntry>::iterator StreamParseLoopRec::erase(list<LoopTraceEntry>::iterator iter){
  loopTraceEntriesSize--;
  return loopTraceEntries.erase(iter);
}

uint64_t StreamParseLoopRec::size(){
  return loopTraceEntriesSize;
}

list<LoopTraceEntry>::const_iterator StreamParseLoopRec::begin() const {
  return loopTraceEntries.begin();
}

list<LoopTraceEntry>::const_iterator StreamParseLoopRec::end() const {
  return loopTraceEntries.end();
}

list<LoopTraceEntry>::iterator StreamParseLoopRec::begin(){
  return loopTraceEntries.begin();
}

list<LoopTraceEntry>::iterator StreamParseLoopRec::end(){
  return loopTraceEntries.end();
}

LoopTraceEntry& StreamParseLoopRec::back(){
  return loopTraceEntries.back();
}

void StreamParseLoopRec::push_back(const LoopTraceEntry& entry){
  loopTraceEntriesSize++;
  loopTraceEntries.push_back(entry);
  lut.insertToLut(entry.getRepetitionPattern(), next(loopTraceEntries.end(), -1));
}

list<LoopTraceEntry>::iterator StreamParseLoopRec::getLoopTraceEntryFromInvocationNumber(uint64_t n){
  auto it = lut.lookupData(n);
  if(it == lut.end())
    return end();
  else
    return lut.getDataFromIterator(it);
}

StreamParseLoopRec::invocation_iterator StreamParseLoopRec::ii_begin(){
  iterator firstInvoc = findLoopTraceEntry(0);
  if(firstInvoc == end())
    return ii_end();
  else
    return invocation_iterator(firstInvoc, 0);
}

StreamParseLoopRec::invocation_iterator StreamParseLoopRec::ii_end(){
  return invocation_iterator(loopTraceEntries.end(), 0);
}

StreamParseLoopRec::invocation_iterator StreamParseLoopRec::ii_next(invocation_iterator& ii){
  /* Clean up the previous invocation if possible, perhaps it's not appropriate for this to be here? */
  if(ii.second + 1 > ii.first->finalInvocationNumber())
    erase(ii.first);

  /* Find the next one */
  iterator nextInvoc = findLoopTraceEntry(ii.second + 1);
  if(nextInvoc == end())
    return ii_end();
  else
    return invocation_iterator(nextInvoc, ii.second + 1);
}

StreamParseLoopRec::iterator StreamParseLoopRec::findLoopTraceEntry(uint64_t invocNum){
  /* Search through the invocation groups that have already been parsed to find a match */
  iterator lteIter = getLoopTraceEntryFromInvocationNumber(invocNum);
  if(lteIter != end()){
    return lteIter;
  }

  /* If no match was found, parse new groups until a match is found */
  LoopTraceEntry entry;
  if(streamer.getNextEntry(entry)){
    push_back(entry);
    back().precomputeDataStructures();
    return next(end(), -1);
  }
  else{
    return end();
  }
}

ostream& operator<<(ostream& os, const LoopTraceEntry& lte){
  os << "{";
  for(auto rangeIter = lte.begin(); rangeIter != lte.end(); rangeIter++){
    os << rangeIter->first;
    if(rangeIter->first != rangeIter->second)
      os << "-" << rangeIter->second;
    if(next(rangeIter, 1) != lte.end())
      os << ",";
  }
  os << "} " << lte.invocation.numIterationGroups() << endl;
  os << lte.invocation;
  os << endl;
  return os;
}

ostream& operator<<(ostream& os, const StreamParseLoopRec& loop){
  for(list<LoopTraceEntry>::const_iterator i = loop.begin(); i != loop.end(); i++){
    os << *i << endl;
  }
  return os;
}

