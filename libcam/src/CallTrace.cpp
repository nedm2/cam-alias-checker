
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

#include <assert.h>
#include "CallTrace.h"
#include <algorithm>
#include <list>
using namespace std;

typedef tuple<uintptr_t, uint64_t, uint64_t> CurrInst;// (callId, currentInstance, currentInstanceIteration)
CurrInst getMinimumIteration(vector<CurrInst> currInsts){
  CurrInst m = currInsts.front();
  for(auto i = currInsts.begin(); i != currInsts.end(); i++){
    if(get<2>(*i) < get<2>(m))
      m = *i;
  }
  return m;
}

/* May invalidate iterators for currInsts */
void updateCurrentInstances(vector<CurrInst>& currInsts, uintptr_t callId, InvocationGroupCfc& invGroup){
  for(auto i = currInsts.begin(); i != currInsts.end(); i++){
    /* Find the entry matching this callID */
    if(get<0>(*i) == callId){
      /* If the current instance is equal to the number of instances in this invocation, we've finished with this call, remove it */
      if(invGroup.getNumInstances(callId) - 1 == get<1>(*i))
        currInsts.erase(i);
      /* Otherwise advance to the next instance */
      else
        *i = CurrInst(callId, get<1>(*i) + 1, invGroup.getIterationNumber(callId, get<1>(*i) + 1));
      break;
    }
  }
}

/* 
 * CallTraceInstanceGroup
*/

CallTraceInstanceGroup::CallTraceInstanceGroup() {}

CallTraceInstanceGroup::CallTraceInstanceGroup(uint64_t instance, vector<CompressionPattern>& cf){
  repetitions.addPair(instance, instance);
  controlFlow = cf;
}

void CallTraceInstanceGroup::addPattern(uint64_t start, uint64_t end){
  repetitions.addPair(start, end);
}

void CallTraceInstanceGroup::addControlFlow(CompressionPattern& cp){
  controlFlow.push_back(cp);
}

bool CallTraceInstanceGroup::isSingleInstance(){
  return repetitions.isSingleton();
}

uint64_t CallTraceInstanceGroup::getFirstInstance(){
  return repetitions.getFirstInstance();
}

uint64_t CallTraceInstanceGroup::getLastInstance(){
  return repetitions.getLastInstance();
}

CallTraceInstanceGroup CallTraceInstanceGroup::extractFirstInstance(){
  uint64_t firstInstance = getFirstInstance();
  repetitions.removeFirstInstance();
  return CallTraceInstanceGroup(firstInstance, controlFlow);
}

CallTraceInstanceGroup CallTraceInstanceGroup::extractLastInstance(){
  uint64_t lastInstance = getLastInstance();
  repetitions.removeLastInstance();
  return CallTraceInstanceGroup(lastInstance, controlFlow);
}

void CallTraceInstanceGroup::buildNumberOfInstancesLut(){
  numInstancesLut.clear();
  for(auto cp = controlFlow.begin(); cp != controlFlow.end(); cp++){
    map<uintptr_t, uint64_t> cpInstances = cp->getNumberOfInstancesMap();
    for(auto i = cpInstances.begin(); i != cpInstances.end(); i++){
      numInstancesLut[i->first] += i->second;
    }
  }
}

map<uintptr_t, uint64_t>& CallTraceInstanceGroup::getNumInstancesLut(){
  return numInstancesLut;
}

RepetitionPattern& CallTraceInstanceGroup::getRepetitionPattern(){
  return repetitions;
}

vector<CompressionPattern>& CallTraceInstanceGroup::getControlFlow(){
  return controlFlow;
}

ostream& operator<<(ostream& os, const CallTraceInstanceGroup& instance){
  cout << instance.repetitions << " ";
  for(auto i = instance.controlFlow.begin(); i !=  instance.controlFlow.end(); i++)
    cout << *i;
  return os;
}

/* 
 * CallTraceCall
*/
void CallTraceCall::addCallInstance(){
  callInstances.push_back(CallTraceInstanceGroup());
}

void CallTraceCall::addCallPattern(uint64_t start, uint64_t end){
  callInstances.back().addPattern(start, end);
}

void CallTraceCall::addCallCfc(CompressionPattern& cp){
  callInstances.back().addControlFlow(cp);
}

void CallTraceCall::addCallTraceCall(CallTraceCall& c){
  auto compareLast = [] (CallTraceInstanceGroup& a, CallTraceInstanceGroup& b) { 
    return a.getLastInstance() < b.getLastInstance();
  };
  auto compareFirst = [] (CallTraceInstanceGroup& a, CallTraceInstanceGroup& b) { 
    return a.getFirstInstance() < b.getFirstInstance();
  };
  bool copiedMinNew = false;
  vector<CallTraceInstanceGroup>::iterator maxExisting;
  vector<CallTraceInstanceGroup>::iterator minNew;
  if(!callInstances.empty() && !c.callInstances.empty()){
    maxExisting = max_element(callInstances.begin(), callInstances.end(), compareLast);
    minNew = min_element(c.callInstances.begin(), c.callInstances.end(), compareFirst);
    if(maxExisting->getLastInstance() == minNew->getFirstInstance()){
      CallTraceInstanceGroup* workingOn;
      if(!maxExisting->isSingleInstance()){
        callInstances.push_back(maxExisting->extractLastInstance());
        workingOn = &callInstances.back();
      }
      else
        workingOn = &*maxExisting;
      /* Add in the overlapping instance's control flow */
      for(auto i = minNew->getControlFlow().begin(); i != minNew->getControlFlow().end(); i++)
        workingOn->addControlFlow(*i);
      copiedMinNew = true;
    }
  }

  /* Add all remaining instances into the original */
  for(auto i = c.callInstances.begin(); i != c.callInstances.end(); i++){
    if(copiedMinNew && i == minNew){
      if(!i->isSingleInstance()){
        i->extractFirstInstance();
        callInstances.push_back(*i);
      }
    }
    else{
      callInstances.push_back(*i);
    }
  }
}

ostream& operator<<(ostream& os, const CallTraceCall& call){
  cout << call.callInstances.size() << endl;
  for(auto i = call.callInstances.begin(); i != call.callInstances.end(); i++)
    cout << *i << endl;
  return os;
}

CallTraceInstanceGroup& CallTraceCall::getInstanceGroupFromInstanceNumber(uint64_t callInstance){
  return *lut.lookupData(callInstance);
}

bool CallTraceCall::empty(){
  return callInstances.empty();
}

void CallTraceCall::buildLut(){
  /* Build lut for looking up call instances */
  vector<pair<RepetitionPattern&, CallTraceInstanceGroup*>> lutInput;
  for(auto i = callInstances.begin(); i != callInstances.end(); i++){
    lutInput.push_back(pair<RepetitionPattern&, CallTraceInstanceGroup*>(i->getRepetitionPattern(), &*i));
  }
  lut.buildLut(lutInput);

  /* Build lut for looking up dynamic instances of instructions */
  for(auto i = callInstances.begin(); i != callInstances.end(); i++)
    i->buildNumberOfInstancesLut();
}

CallTraceCall::iterator CallTraceCall::callInstanceIteratorBegin(){
  return lut.begin();
}

CallTraceCall::iterator CallTraceCall::callInstanceIteratorNext(CallTraceCall::iterator iter){
  return lut.next(iter);
}

CallTraceCall::iterator CallTraceCall::callInstanceIteratorEnd(){
  return lut.end();
}

/* 
 * CallTraceLoopInvocationGroup
*/

CallTraceLoopInvocationGroup::CallTraceLoopInvocationGroup() : callTraceCacheBuilt(false) {}

CallTraceLoopInvocationGroup::CallTraceLoopInvocationGroup(uint64_t n, map<uintptr_t, CallTraceCall> c){
  addInvocationPattern(n, n);
  calls = c;
  callTraceCacheBuilt = false;
}

void CallTraceLoopInvocationGroup::addInvocationPattern(uint64_t start, uint64_t end){
  repetitions.addPair(start, end);
}

void CallTraceLoopInvocationGroup::addNewCall(uintptr_t callID){
  /* This doesn't actually do anything because looking up a map with a given
   * Call ID will automatically add a call to the map */ 
}

void CallTraceLoopInvocationGroup::addCallInstance(uintptr_t callID){
  calls[callID].addCallInstance();
}

void CallTraceLoopInvocationGroup::addCallPattern(uintptr_t callID, uint64_t start, uint64_t end){
  calls[callID].addCallPattern(start, end);
}

void CallTraceLoopInvocationGroup::addCallCfc(uintptr_t callID, CompressionPattern& cp){
  calls[callID].addCallCfc(cp);
}

CallTraceInstanceGroup& CallTraceLoopInvocationGroup::getInstanceGroupFromInstanceNumber(uintptr_t callID, uint64_t callInstance){
  return calls[callID].getInstanceGroupFromInstanceNumber(callInstance);
}

uint64_t CallTraceLoopInvocationGroup::getFirstInvocationNumber(){
  return repetitions.getFirstInstance();
}

uint64_t CallTraceLoopInvocationGroup::getLastInvocationNumber(){
  return repetitions.getLastInstance();
}

bool CallTraceLoopInvocationGroup::isASingleInvocation(){
  return repetitions.isSingleton();
}

CallTraceLoopInvocationGroup CallTraceLoopInvocationGroup::extractFirstInvocationIntoNewGroup(){
  CallTraceLoopInvocationGroup newGroup(getFirstInvocationNumber(), calls);
  repetitions.removeFirstInstance();
  return newGroup;
}

CallTraceLoopInvocationGroup CallTraceLoopInvocationGroup::extractLastInvocationIntoNewGroup(){
  CallTraceLoopInvocationGroup newGroup(getLastInvocationNumber(), calls);
  repetitions.removeLastInstance();
  return newGroup;
}

bool CallTraceLoopInvocationGroup::isCallTraceCacheBuilt(){
  return callTraceCacheBuilt;
}

void CallTraceLoopInvocationGroup::addToCallTraceCache(uintptr_t callId, uint64_t callInstance, CallTraceInstanceGroup& callInvoc, map<uintptr_t, uint64_t>& runningCounts){
  /* For each sub instruction in this call instance, add the number of dynamic instances to the cache */
  const map<uintptr_t, uint64_t>& numInstances = callInvoc.getNumInstancesLut();
  for(auto instr = numInstances.begin(); instr != numInstances.end(); instr++){
    callTraceCache[instr->first].push_back(CallTraceInstructionCacheEntry(
      runningCounts[instr->first], 
      runningCounts[instr->first] + instr->second - 1,
      callId,
      callInstance));
    runningCounts[instr->first] += instr->second;
  }
}

void CallTraceLoopInvocationGroup::buildCallTraceCache(InvocationGroupCfc &invGroup, 
    map<uintptr_t, uint64_t>& subInstrRunningCount, set<uintptr_t>& callInstrList){
  vector<CurrInst> currentInstances; 
  map<uintptr_t, uint64_t> runningCounts;

  /* Build list of initial instances of each call instruction to step through the trace */
  for(auto i = callInstrList.begin(); i != callInstrList.end(); i++){
    if(!calls[*i].empty() && invGroup.containsInstruction(*i)){
      uint64_t firstInstance = 0;
      uint64_t iterationOfFirstInstance = invGroup.getIterationNumber(*i, firstInstance);
      currentInstances.push_back(CurrInst(*i, firstInstance, iterationOfFirstInstance));
    }
  }

  /* Step through the trace accounting for all sub instruction in each call in order */
  while(currentInstances.size() > 0){
    CurrInst minimumIteration = getMinimumIteration(currentInstances);
    CallTraceInstanceGroup& callInvoc = getInstanceGroupFromInstanceNumber(get<0>(minimumIteration), get<1>(minimumIteration));
    addToCallTraceCache(get<0>(minimumIteration), get<1>(minimumIteration), callInvoc, runningCounts);
    updateCurrentInstances(currentInstances, get<0>(minimumIteration), invGroup);
  }
  for(auto i = runningCounts.begin(); i != runningCounts.end(); i++){
    subInstrRunningCount[i->first] += i->second;
  }

  callTraceCacheBuilt = true;

  /* Build constantCallIDCache */
  for(auto i = subInstrRunningCount.begin(); i != subInstrRunningCount.end(); i++){
    if(callTraceCache.find(i->first) != callTraceCache.end()){
      constantCallIDCache[i->first] = true;
      uintptr_t callID = get<2>(callTraceCache[i->first].front());
      for(auto entry = callTraceCache[i->first].begin(); entry != callTraceCache[i->first].end(); entry++){
        if(get<2>(*entry) != callID){
          constantCallIDCache[i->first] = false;
          break;
        }
      }
    }
  }
}

void CallTraceLoopInvocationGroup::printCallTraceCache(){
  for(auto i = callTraceCache.begin(); i != callTraceCache.end(); i++){
    cout << i->first << endl;
    for(auto j = i->second.begin(); j != i->second.end(); j++){
      cout << "(" << get<0>(*j) << "->" << get<1>(*j) << ": " << get<2>(*j) << "[" << get<3>(*j) << "]" << ")" << endl;
    }
    cout << endl;
  }
}

CallTraceLoopInvocationGroup::ci_iterator CallTraceLoopInvocationGroup::callInstanceIteratorBegin(uintptr_t instrID){
  return callTraceCache[instrID].begin();
}

CallTraceLoopInvocationGroup::ci_iterator CallTraceLoopInvocationGroup::callInstanceIteratorNext(ci_iterator iter){
  return next(iter, 1);
}

CallTraceLoopInvocationGroup::ci_iterator CallTraceLoopInvocationGroup::callInstanceIteratorEnd(uintptr_t instrID){
  return callTraceCache[instrID].end();
}

uint64_t CallTraceLoopInvocationGroup::getCallInstanceFromCII(ci_iterator cii){
  return get<3>(*cii);
}

uintptr_t CallTraceLoopInvocationGroup::getCallIDFromCII(ci_iterator cii){
  return get<2>(*cii);
}

/* Number of sub instruction instances for this call instance */
uint64_t CallTraceLoopInvocationGroup::getNumInstancesFromCII(ci_iterator cii){
  return get<1>(*cii) - get<0>(*cii) + 1;
}

bool CallTraceLoopInvocationGroup::isConstantCallID(uintptr_t callID){
  assert(constantCallIDCache.find(callID) != constantCallIDCache.end());
  return constantCallIDCache[callID];
}

void CallTraceLoopInvocationGroup::buildLuts(){
  for(auto i = calls.begin(); i != calls.end(); i++)
    i->second.buildLut();
}

RepetitionPattern& CallTraceLoopInvocationGroup::getRepetitionPattern(){
  return repetitions;
}

bool CallTraceLoopInvocationGroup::isEmpty(){
  return calls.empty() && repetitions.empty();
}

void CallTraceLoopInvocationGroup::clear(){
  calls.clear();
  callTraceCache.clear();
  callTraceCacheBuilt = false;
  repetitions.clear();
}

uint64_t CallTraceLoopInvocationGroup::getNumInstances(uint64_t subInstrID){
  /* Find the last entry in the cache for this instruction and get the end instance of that entry */
  if(callTraceCache[subInstrID].empty())
    return 0;
  else
    return get<1>(callTraceCache[subInstrID].back()) + 1;
}

bool CallTraceLoopInvocationGroup::lastAndFirstInvocationOverlap(CallTraceLoopInvocationGroup& other){
  return repetitions.getLastInstance() == other.repetitions.getFirstInstance();
}

void CallTraceLoopInvocationGroup::mergeCallInfo(map<uintptr_t, CallTraceCall>& c){
  for (auto call = c.begin(); call != c.end(); call++){
    calls[call->first].addCallTraceCall(call->second);
  }
}

vector<CallTraceLoopInvocationGroup> CallTraceLoopInvocationGroup::mergeIntoAndReturnRemaining(CallTraceLoopInvocationGroup& other){
  vector<CallTraceLoopInvocationGroup> newGroups;

  /* If there is no overlap, no changes need be made */
  if(other.isEmpty() || !lastAndFirstInvocationOverlap(other)){
    if(!other.isEmpty())
      newGroups.push_back(other);
    return newGroups;
  }

  assert(getLastInvocationNumber() == other.getFirstInvocationNumber());

  CallTraceLoopInvocationGroup* workingOn = this;

  /* If this is group has multiple invocations, split off the previous ones and add a new group with just this invocation */
  if(!isASingleInvocation()){
    newGroups.push_back(extractLastInvocationIntoNewGroup());
    workingOn = &newGroups.back();
  }

  workingOn->mergeCallInfo(other.calls);

  /* If the other group had more than one invocation, the merged invocation should be removed from that group */
  if(!other.isASingleInvocation()){
    other.extractFirstInvocationIntoNewGroup();
    newGroups.push_back(other);
  }

  return newGroups;
}

ostream& operator<<(ostream& os, const CallTraceLoopInvocationGroup& inv){
  cout << inv.repetitions << " " << inv.calls.size() << endl;
  for(auto i = inv.calls.begin(); i != inv.calls.end(); i++)
    cout << i->first << " " << i->second << endl;
  return os;
}

/* 
 * StreamParseCallTrace 
*/

void StreamParseCallTrace::addNewInvocationGroup(CallTraceLoopInvocationGroup& invGroup){
  invocations.push_back(invGroup);
}

CallTraceLoopInvocationGroup& StreamParseCallTrace::getInvocCallTraceFromInvocNumber(uint64_t invocNum){
  return *lut.lookupData(invocNum);
}

bool StreamParseCallTrace::empty(){
  for(auto i = invocations.begin(); i != invocations.end(); i++)
    if(!i->isEmpty())
      return false;
  return true;
}

void StreamParseCallTrace::buildLuts(){
  vector<pair<RepetitionPattern&, CallTraceLoopInvocationGroup*>> lutInput;
  for(auto i = invocations.begin(); i != invocations.end(); i++){
    i->buildLuts();
    lutInput.push_back(pair<RepetitionPattern&, CallTraceLoopInvocationGroup*>(i->getRepetitionPattern(), &*i));
  }
  lut.buildLut(lutInput);
}

void StreamParseCallTrace::printLut(){
  cout << lut << endl;
}

ostream& operator<<(ostream& os, const StreamParseCallTrace& trace){
  cout << trace.invocations.size() << endl;
  for(auto i = trace.invocations.begin(); i != trace.invocations.end(); i++){
    cout << *i << endl;
  }
  return os;
}

#if 0
map<uintptr_t, CallTrace::iterator> CallTraces::sliceIterators;
map<uintptr_t, uint64_t> CallTraces::outstandingCallInstances;
#endif
