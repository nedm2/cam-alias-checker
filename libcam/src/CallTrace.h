
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

#ifndef CALL_TRACE_H
#define CALL_TRACE_H

#include "cam_system.h"
#include "StaticLoopRec.h"
#include "RepetitionPattern.h"
#include <map>
#include <vector>
#include <iostream>
#include <tuple>
#include <set>
using namespace std;

class CallTraceInstanceGroup{
  RepetitionPattern repetitions;
  vector<CompressionPattern> controlFlow;

  map<uintptr_t, uint64_t> numInstancesLut;

public:
  CallTraceInstanceGroup();
  CallTraceInstanceGroup(uint64_t instance, vector<CompressionPattern>& cf);

  void addPattern(uint64_t start, uint64_t end);
  void addControlFlow(CompressionPattern& cp);
  RepetitionPattern& getRepetitionPattern();
  vector<CompressionPattern>& getControlFlow();
  bool isSingleInstance();
  uint64_t getFirstInstance();
  uint64_t getLastInstance();
  CallTraceInstanceGroup extractLastInstance();
  CallTraceInstanceGroup extractFirstInstance();

  void buildNumberOfInstancesLut();
  map<uintptr_t, uint64_t>& getNumInstancesLut();

  friend ostream& operator<<(ostream& os, const CallTraceInstanceGroup& instance);
};

class CallTraceCall{
  vector<CallTraceInstanceGroup> callInstances;
  RepetitionPatternLookup<CallTraceInstanceGroup> lut;

public:
  void addCallInstance();
  void addCallPattern(uint64_t start, uint64_t end);
  void addCallCfc(CompressionPattern& cp);
  void addCallTraceCall(CallTraceCall& c);

  void buildLut();
  CallTraceInstanceGroup& getInstanceGroupFromInstanceNumber(uint64_t callInstance);

  bool empty();

  typedef RepetitionPatternLookup<CallTraceInstanceGroup>::iterator iterator;
  iterator callInstanceIteratorBegin();
  iterator callInstanceIteratorNext(iterator iter);
  iterator callInstanceIteratorEnd();

  friend ostream& operator<<(ostream& os, const CallTraceCall& call);
};

class CallTraceLoopInvocationGroup{
  RepetitionPattern repetitions;
  map<uintptr_t, CallTraceCall> calls;

public:
  /* (Start instance, end instance, callID, call instance) */
  class CallTraceInstructionCacheEntry : public tuple<uint64_t, uint64_t, uintptr_t, uint64_t>{
  public:
    CallTraceInstructionCacheEntry(uint64_t start, uint64_t end, uintptr_t callId, uint64_t callInst)
      : tuple<uint64_t, uint64_t, uintptr_t, uint64_t>(start, end, callId, callInst) {}
    friend bool operator < (const CallTraceInstructionCacheEntry & left, uint64_t val) { return get<1>(left) < val; }
    friend bool operator < (uint64_t val, const CallTraceInstructionCacheEntry & right) { return val < get<1>(right); }
  };
private:
  map<uintptr_t, vector<CallTraceInstructionCacheEntry>> callTraceCache;
  map<uintptr_t, bool> constantCallIDCache;
  bool callTraceCacheBuilt;

  void mergeCallInfo(map<uintptr_t, CallTraceCall>& c);

public:
  CallTraceLoopInvocationGroup();
  CallTraceLoopInvocationGroup(uint64_t n, map<uintptr_t, CallTraceCall> c);

  void addInvocationPattern(uint64_t start, uint64_t end);
  void addNewCall(uintptr_t callID);
  void addCallInstance(uintptr_t callID);
  void addCallPattern(uintptr_t callID, uint64_t start, uint64_t end);
  void addCallCfc(uintptr_t callID, CompressionPattern& cp);
  RepetitionPattern& getRepetitionPattern();
  uint64_t getFirstInvocationNumber(); 
  uint64_t getLastInvocationNumber(); 
  bool isASingleInvocation();
  CallTraceLoopInvocationGroup extractLastInvocationIntoNewGroup();
  CallTraceLoopInvocationGroup extractFirstInvocationIntoNewGroup();

  bool isCallTraceCacheBuilt();
  void buildCallTraceCache(InvocationGroupCfc &invGroup, map<uintptr_t, uint64_t>& subInstrRunningCount, set<uintptr_t>& callInstrList);
  void addToCallTraceCache(uintptr_t callId, uint64_t callInstance, CallTraceInstanceGroup& callInvoc, map<uintptr_t, uint64_t>& subInstrRunningCount);
  void printCallTraceCache();
  void buildLuts();

  CallTraceInstanceGroup& getInstanceGroupFromInstanceNumber(uintptr_t callID, uint64_t callInstance);

  /* Iterator for traversing the call trace cache */
  typedef vector<CallTraceInstructionCacheEntry>::iterator ci_iterator;
  ci_iterator callInstanceIteratorBegin(uintptr_t instrID);
  ci_iterator callInstanceIteratorNext(ci_iterator iter);
  ci_iterator callInstanceIteratorEnd(uintptr_t instrID);
  uint64_t getCallInstanceFromCII(ci_iterator cii);
  uintptr_t getCallIDFromCII(ci_iterator cii);
  uint64_t getNumInstancesFromCII(ci_iterator cii); /* Number of sub instruction instances for this call instance */

  /* Return true if every instance of this sub instruction came from the same call ID */
  bool isConstantCallID(uintptr_t callID);

  bool isEmpty();
  void clear();
  bool lastAndFirstInvocationOverlap(CallTraceLoopInvocationGroup& other);

  /* Return the total number of instances of a sub instruction in this loop invocation */
  uint64_t getNumInstances(uint64_t subInstrID);

  /* 
   * Merges overlapping invocations (like when the call trace is dumped into several files). Assume only
   * one invocation can be shared between two groups (the last in one group and the first in the next).
  */
  vector<CallTraceLoopInvocationGroup> mergeIntoAndReturnRemaining(CallTraceLoopInvocationGroup& other);


  friend ostream& operator<<(ostream& os, const CallTraceLoopInvocationGroup& inv);
};

class StreamParseCallTrace{
  list<CallTraceLoopInvocationGroup> invocations;
  RepetitionPatternLookup<CallTraceLoopInvocationGroup> lut;

public:
  void addNewInvocationGroup(CallTraceLoopInvocationGroup& invGroup);
  void buildLuts();
  void printLut();
  bool empty();
  CallTraceLoopInvocationGroup& getInvocCallTraceFromInvocNumber(uint64_t invocNum);
  friend ostream& operator<<(ostream& os, const StreamParseCallTrace& trace);
};

#endif
