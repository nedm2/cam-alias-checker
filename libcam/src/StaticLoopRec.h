
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

#ifndef STATIC_LOOP_REC_H
#define STATIC_LOOP_REC_H

#include <cstdint>
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <functional>
#include <numeric>
#include <algorithm>
#include <tuple>
#include <deque>

#include "CompressionPattern.h"
#include "RepetitionPattern.h"
#include "LoopTraceStreamer.h"

using namespace std;

class IterationGroupCfc{
  RepetitionPattern repetitions;
  vector<CompressionPattern> controlFlow;
  map<uintptr_t, uint64_t> numInstancesLut;

public:
  IterationGroupCfc();
  IterationGroupCfc(uint64_t n, vector<CompressionPattern>& cf);

  /* Return true if the no instructions were recorded for this iteration */
  bool isEmpty() const ;

  /* Return the number of instances of a particular instruction in a single iteration of this group */
  uint64_t getNumberOfInstancesInSingleIteration(uintptr_t instrID);

  /* Build a table of how many instances of each instruction we have in one invocation */
  void buildNumberOfInstancesLut();

  uint64_t getFirstIteration();
  uint64_t getLastIteration();
  bool isSingleIteration();
  IterationGroupCfc extractFirstIteration();
  IterationGroupCfc extractLastIteration();

  RepetitionPattern& getRepetitionPattern();
  vector<CompressionPattern>& getControlFlow();

  /* Expose iterators for the repetition pattern */
  vector<pair<uint64_t, uint64_t>>::iterator patternIterBegin();
  vector<pair<uint64_t, uint64_t>>::iterator patternIterEnd();

  uint64_t getNumberOfIterations_slow();

  const map<uintptr_t, uint64_t>& getNumInstancesLut();

  void addIterationPattern(uint64_t startIteration, uint64_t endIteration);
  void addIterationPattern(uint64_t iteration);
  void addIntoControlFlow(CompressionPattern& cp);
  friend ostream& operator<<(ostream& os, const IterationGroupCfc& invGroup);
};

class InvocationGroupCfc {

public:
  /* (start iter, end iter, IterationGroupCfc*) */
  //typedef tuple<uint64_t, uint64_t, IterationGroupCfc*> IterGroupLutEntry; 

  /* (start instance, end instance, iteration pattern start, num instances per iteration) */
  typedef tuple<uint64_t, uint64_t, uint64_t, uint64_t> InstructionInstanceLutEntry;

private:
  vector<IterationGroupCfc> iterations;
  map<uintptr_t, uint64_t> invocationInstancesLut;

  /* 
   * LUT to find iteration from the instance number of a dynamic instruction, for each 
   * instruction ID we have a vector of:
   * (start dynamic instance, end dynamic instance, iteration number of first instance, number of instances per iteration)
  */
  map<uintptr_t, vector<InstructionInstanceLutEntry>> instructionInstanceLut;

  /* LUT mapping ranges of iteration numbers to IterationGroupCfc pointers */
  RepetitionPatternLookup<IterationGroupCfc> iterLut;

  /* Build a LUT in each IterationGroupCfc for looking up number of instances of an instruction */
  void buildIterationNumInstancesLuts();

  /* Build a LUT for finding IterationGroupCfc from iteration number */
  void buildIterGroupLut();

  /* Build a LUT of the total instances of an instruction in an invocation */
  void buildInvocationNumInstancesLut();

  /* Build a LUT for each instruction ID for looking up iteration numbers from instruction instance */
  void buildInstructionInstanceLut();

public:

  /* Return true if the no instructions were recorded for this invocation */
  bool isEmpty() const ;

  /* Do all data structure precomputations */
  void precomputeDataStructures();
  void printNumInstancesLut();
  void printInstructionInstancesLut();
  void printIterLut();

  uint64_t getIterationNumber(uintptr_t instrID, uint64_t instance);

  bool containsInstruction(uintptr_t instrID);

  uint64_t numIterationGroups() const;

  void mergeIterationInfo(InvocationGroupCfc& inv);

  /* Return the total number of instances of an instruction in one invocation */
  uint64_t getNumInstances(uintptr_t instrID);
  
  /* Iterators for iterating across the iterations */
  IterationGroupCfc* getIterationFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii);
  uint64_t getNumIterationsFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii);
  uint64_t getStartIterationFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii);
  uint64_t getNumInstancesFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii, uintptr_t instrID);
  uint64_t getIterationNumberFromII(RepetitionPatternLookup<IterationGroupCfc>::iterator ii, uintptr_t instrID);
  RepetitionPatternLookup<IterationGroupCfc>::iterator iterationIteratorBegin();
  RepetitionPatternLookup<IterationGroupCfc>::iterator iterationIteratorNext(RepetitionPatternLookup<IterationGroupCfc>::iterator ii);
  RepetitionPatternLookup<IterationGroupCfc>::iterator iterationIteratorEnd();

  /* Lookup an iteration group from iteration number, should be O(log(n)) in number of iterations */
  IterationGroupCfc* getIterationGroupFromIterationNumber(uint64_t n);

  void clear();

  uint64_t getNumberOfIterations();

  IterationGroupCfc &addIterationGroup();
  void addIterationPattern(uint64_t startIteration, uint64_t endIteration);
  void addIterationPattern(uint64_t iteration);
  void addCfc(CompressionPattern &cp);
  friend ostream& operator<<(ostream& os, const InvocationGroupCfc& invGroup);
};

class LoopTraceEntry {
  RepetitionPattern invocationRepetitionPattern;
  InvocationGroupCfc invocation;
public:
  LoopTraceEntry();
  LoopTraceEntry(uint64_t n, InvocationGroupCfc& inv);

  static bool compareRange(const pair<uint64_t, uint64_t> & left, uint64_t val);

  /* Return the invocation number of the last invocation which uses this pattern */
  uint64_t finalInvocationNumber();

  /* Add a new compression pattern to the compressed trace */
  void addCfc(CompressionPattern& cp);

  /* Add a new range of invocation/iteration numbers described by this pattern */
  void addInvocationPattern(uint64_t start, uint64_t end);
  void addInvocationPattern(uint64_t invoc);
  void addIterationPattern(uint64_t start, uint64_t end);
  void addIterationPattern(uint64_t iter);
  void addIterationGroup();
  const RepetitionPattern& getRepetitionPattern() const;

  uint64_t getFirstInvocationNumber();
  uint64_t getLastInvocationNumber();
  bool isASingleInvocation();
  LoopTraceEntry extractFirstInvocationIntoNewGroup();
  LoopTraceEntry extractLastInvocationIntoNewGroup();

  /* Do all data structure precomputations */
  void precomputeDataStructures();
  
  friend ostream& operator<<(ostream& os, const LoopTraceEntry& lte);

  /* Return a pointer to the InvocationGroupCfc */
  InvocationGroupCfc* getInvocationGroupPointer();

  /* Return true if last invocation number of this invocation equals first invocation number of other */
  bool lastAndFirstInvocationOverlap(LoopTraceEntry& other);

  /* 
   * Merges overlapping invocations (like when the loop trace is dumped into several files). Assume only
   * one invocation can be shared between two groups (the last in one group and the first in the next).
  */
  vector<LoopTraceEntry> mergeIntoAndReturnRemaining(LoopTraceEntry& other);

  uint64_t getNumberOfInvocations_slow();

  void clear();
  bool isEmpty();

  /* Expose some of the invocationRepetitionPattern interface */
  vector<pair<uint64_t, uint64_t>>::iterator begin();
  vector<pair<uint64_t, uint64_t>>::iterator end();
  vector<pair<uint64_t, uint64_t>>::const_iterator begin() const ;
  vector<pair<uint64_t, uint64_t>>::const_iterator end() const ;
};

class StreamParseLoopRec{
  /* Must be list, relies on non-invalidation of iterators on insert/erase */
  list<LoopTraceEntry> loopTraceEntries;
  uint64_t loopTraceEntriesSize; /* standard implementation of list::size() is O(n) for some reason */
  RepetitionPatternSortedLookup<list<LoopTraceEntry>::iterator> lut;
  LoopTraceStreamer streamer;
public:
  StreamParseLoopRec();

  friend ostream& operator<<(ostream& os, const StreamParseLoopRec& loops);

  typedef list<LoopTraceEntry>::iterator iterator;

  list<LoopTraceEntry>::iterator erase(list<LoopTraceEntry>::iterator iter);
  uint64_t size();
  list<LoopTraceEntry>::const_iterator begin() const ;
  list<LoopTraceEntry>::const_iterator end() const ;
  list<LoopTraceEntry>::iterator begin();
  list<LoopTraceEntry>::iterator end();
  LoopTraceEntry& back();
  void push_back(const LoopTraceEntry& entry);

  typedef pair<iterator, uint64_t> invocation_iterator;
  invocation_iterator ii_begin();
  invocation_iterator ii_end();
  invocation_iterator ii_next(invocation_iterator& ii);
  iterator findLoopTraceEntry(uint64_t invocNum);

  /* Searches for the LoopTraceEntry by invocation number, returns NULL if
   * no record for this number has been found */
  list<LoopTraceEntry>::iterator getLoopTraceEntryFromInvocationNumber(uint64_t n);
};

#endif
