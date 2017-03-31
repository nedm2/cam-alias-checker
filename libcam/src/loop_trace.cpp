/*
 * Copyright (C) 2014  Simone Campanoni, Timothy M Jones, Niall Murphy
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
#include <errno.h>
#include <stdio.h>
#include <xanlib.h>
#include <bzlib.h>
#include "cam.h"
#include "cam_system.h"
#include "loop_trace.hh"
#include "memory_allocator.hh"
#include "ControlFlowCompressor.h"
#include "TimeoutCounter.h"
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
using namespace std;


/**
 * Maximum amount of memory to use before dumping the trace.
 **/
#define LIBCAM_DEFAULT_MAX_MEM_USAGE ((JITUINT64)1073741824ULL)
//#define LIBCAM_DEFAULT_MAX_MEM_USAGE ((JITUINT64)10000ULL)

/**
 * Maximum number of iterations to check for matches.
 **/
#define MAX_RECENT_ITER_MATCHES 32

/**
 * Maximum number of recent LoopInvocationCallInfos.
 **/
#define MAX_RECENT_LOOP_INV_CALL_INFOS 32

/**
 * Length of compression window (longer means possibly better compression but definitely poorer performance).
 **/
#define COMPRESSION_WINDOW_SIZE 50


/**
 * Convert from an integer to pointer and back.
 **/
#define intToPtr(val) ((void *)(val))
#define uintToPtr(val) ((void *)(val))
#define ptrToInt(ptr) ((JITNINT)(ptr))
#define ptrToUint(ptr) ((JITNUINT)(ptr))


/**
 * An invocation of a currently running structure.
 **/
struct RunningStruct
{
  ExecTrace *trace;
  JITUINT64 invocationNum;
  XanHashTable *currInvocation;
  RunningStruct *next;
  ControlFlowCompressor* currCFC;
  bool partiallyDumped;

  RunningStruct();
  virtual ~RunningStruct();

  /* Record an instruction as seen in this loop. */
  virtual void seenInstruction(JITNINT instID) = 0;

  /* Record all instructions seen on an invocation. */
  virtual void recordInstsSeenOnInvocation(bool attemptMatch) = 0;

  /* Do all that's need to be done to finish an invocation. */
  virtual void finishInvocation(bool attemptMatch) = 0;

  virtual void displaySummary() = 0;

  virtual bool isCall() = 0;
};


/** 
 * An invocation of a currently running loop.
 **/
struct RunningLoop : public RunningStruct
{
  JITUINT64 iterationNum;
  XanList *iterationInfos; // List of IterationInfo pointers, each storing a CFC and list of sharers
  list<IterationInfo*> recentIterationMatches; // LRU cache of IterationInfos for quicker matching

  RunningLoop();
  ~RunningLoop();

  void deleteIterationInfos();

  /* Record all instructions seen on an iteration. */
  void recordInstsSeenOnIteration(void);

  /* Record an instruction as seen in this loop. */
  void seenInstruction(JITNINT instID);

  /* Do all that's need to be done to finish an invocation. */
  void finishInvocation(bool attemptMatch);

  void recordInstsSeenOnInvocation(bool attemptMatch);

  void displaySummary();

  bool isCall(){ return false; }
};


/**
 * An invocation of a currently running call.
 **/
struct RunningCall : public RunningStruct
{
  /* Record an instruction as seen in this loop. */
  void seenInstruction(JITNINT instID);

  /* Do all that's need to be done to finish an invocation. */
  void finishInvocation(bool attemptMatch);

  void recordInstsSeenOnInvocation(bool attemptMatch);

  void displaySummary();

  bool isCall(){ return true; }
};


/**
 * Globals for this pass are all held in a single structure.
 **/
class PassGlobals : public CamMemoryAllocator
{
public:

  XanList *loopInvCallInfos;     /**< List of LoopInvocationCallInfo */
  XanHashTable *loopTraces;      /**< Map from loop ID to a loop trace. */
  XanHashTable *callTraces;      /**< Map from call inst ID to a call trace. */
  XanStack *runningStack;        /**< A stack of running loops and calls. */
  RunningLoop *runningLoopPool;  /**< A pool of free running loop structures. */
  RunningCall *runningCallPool;  /**< A pool of free running call structures. */
  JITUINT64 dumpTraceMemUsage;   /**< Memory size trigger for dumping the trace. */
  JITUINT32 traceDumpID;         /**< An ID for each trace dump that is made. */
  ControlFlowCompressor currIterCfc;
  //ofstream instrTraceFile;
  FILE *loopOutputFile;
  FILE *callOutputFile;
  BZFILE *loopCompressedFile;
  BZFILE *callCompressedFile;
  string outputDirectory;
  bool waitingForInvocCompletion;
  list<LoopInvocationCallInfo*> recentLoopInvCallInfos;

  PassGlobals()
    : runningLoopPool(NULL), runningCallPool(NULL),
      dumpTraceMemUsage(LIBCAM_DEFAULT_MAX_MEM_USAGE), traceDumpID(0), currIterCfc(this, COMPRESSION_WINDOW_SIZE),
      loopCompressedFile(NULL), callCompressedFile(NULL), outputDirectory("."), waitingForInvocCompletion(false)
  {
    //instrTraceFile.open("instruction_trace.txt");
    loopTraces = allocHashTable();
    callTraces = allocHashTable();
    loopInvCallInfos = allocList();
    runningStack = allocStack();
    stackPush(runningStack, NULL);
    char *env = getenv("LIBCAM_LOOP_TRACE_MAX_MEM_USAGE");
    if (env) {
      dumpTraceMemUsage = atoi(env);
    }
    env = getenv("LIBCAM_OUTPUT_DIRECTORY");
    if (env) {
      outputDirectory = env;
    }
  }

  ~PassGlobals()
  {
    //instrTraceFile.close();
    deleteTraceMap<LoopTrace>(loopTraces);
    deleteTraceMap<CallTrace>(callTraces);
    deleteAllLoopInvCallTraces();
    freeList(loopInvCallInfos);
    deleteRunningPool<RunningLoop>(runningLoopPool);
    deleteRunningPool<RunningCall>(runningCallPool);
    freeStack(runningStack);
  }

  /* Fetch a new running loop or call. */
  template<class Running> Running *fetchNewRunningStruct(Running **runningPool);

  /* Fetch a new running loop. */
  RunningLoop *
  fetchNewRunningLoop(void)
  {
    return fetchNewRunningStruct(&runningLoopPool);
  }

  /* Fetch a new running call. */
  RunningCall *fetchNewRunningCall(void)
  {
    return fetchNewRunningStruct(&runningCallPool);
  }

  /* Add a running loop or call back into the pool. */
  void freeRunningStruct(RunningStruct **runningPool, RunningStruct *running);

  /* Add a running loop back into the pool. */
  void
  freeRunningLoop(RunningLoop *running)
  {
    freeRunningStruct((RunningStruct **)&runningLoopPool, running);
  }

  /* Add a running call back into the pool. */
  void
  freeRunningCall(RunningCall *running)
  {
    freeRunningStruct((RunningStruct **)&runningCallPool, running);
  }

  /* Delete a pool of running structures. */
  template<class Running> void deleteRunningPool(Running *runningPool);

  /* Delete a hash table of traces. */
  template<class Trace> void deleteTraceMap(XanHashTable *traces);

  /* Write out traces, if there are any. */
  void writeTraces(string id = "");

  /* Write traces to a file. */
  void writeTracesToFile(BZFILE *compressedFile, XanHashTable *traces);

  /* Dump traces if too much memory has been allocated. */
  void checkDumpTraces(void);
  void dumpTraces();

  /* Check if there is a loop or call running */
  bool loopRunning();
  bool callRunning();

  /* Record the calls for this loop invocation, return true if callTraces should be deleted before reallocation */
  bool recordCallTracesForInvocation(uint64_t invNum, bool attemptMatch);

  uint64_t getCurrentLoopInvocationNumber();

  void clearAllLoopInvCallTraces(bool keepFinal){
    XanListItem* i = xanList_first(loopInvCallInfos);
    while(i){
      if(!(keepFinal && i->next == NULL))
        ((LoopInvocationCallInfo*)i->data)->deleteCallTraceTable();
      deleteMem<LoopInvocationCallInfo>((LoopInvocationCallInfo*)i->data);
      i = i->next;
    }
  }

  void deleteAllLoopInvCallTraces(){
    XanListItem* i = xanList_first(loopInvCallInfos);
    while(i){
      ((LoopInvocationCallInfo*)i->data)->deleteCallTraceTable();
      deleteMem<LoopInvocationCallInfo>((LoopInvocationCallInfo*)i->data);
      i = i->next;
    }
  }

  void openCompressedFiles();
  void closeCompressedFiles();
  bool compressedFilesOpen();
  void writeDumpIncompleteSymbol(BZFILE* compressedFile);
  void writeDumpCompleteSymbol(BZFILE* compressedFile);
  string getOutputDirectory() { return outputDirectory; }
};


/**
 * The globals for this pass.
 **/
static PassGlobals *globals = NULL;
static TimeoutCounter *timeoutCounter = NULL;


/**
 * Get a 64 bit unsigned pointer that can be used as a key into a hash table.
 **/
static inline void *
keyUint64AsPtr(JITUINT64 *uint)
{
  if (sizeof(void *) == 8) {
    return (void *)(*uint);
  } else {
    return (void *)uint;
  }
}


/**
 * Hash function for a 64 bit unsigned integer.
 **/
static inline unsigned int
hashUint64AsPtr(void *uint)
{
  JITUINT64 u;
  if (sizeof(void *) == 8) {
    u = (JITUINT64)uint;
  } else {
    u = *((JITUINT64 *)uint);
  }
  return (u >> 32) | (u & 0xffffffff);
}


/**
 * Compare two 64 bit unsigned integers.
 **/
static inline int
matchUint64AsPtr(void *uint1, void *uint2)
{
  if (sizeof(void *) == 8) {
    return uint1 == uint2;
  } else {
    return *((JITUINT64 *)uint1) == *((JITUINT64 *)uint2);
  }
}


/**
 * Create a 64 bit unsigned integer on the heap.
 **/
static inline JITUINT64 *
createUint64AsPtr(JITUINT64 uint)
{
  if (sizeof(void *) == 8) {
    return (JITUINT64 *)uint;
  } else {
    JITUINT64 *u = (JITUINT64 *)globals->allocMem(sizeof(JITUINT64));
    *u = uint;
    return u;
  }
}


/**
 * Free a 64 bit unsigned integer from the heap.
 **/
static inline void
freeUint64AsPtr(void *u)
{
  if (sizeof(void *) != 8) {
    globals->freeMem(u);
  }
}


/**
 * Get a 64 bit unsigned integer from a pointer.
 **/
static inline JITUINT64
getUint64AsPtr(void *u)
{
  if (sizeof(void *) == 8) {
    return (JITUINT64)u;
  } else {
    return *(JITUINT64 *)u;
  }
}

void RunningLoop::displaySummary(){
  cerr << "Loop" << endl;
}

void RunningCall::displaySummary(){
  cerr << "Call" << endl;
}

IterationInfo::IterationInfo(ControlFlowCompressor *cfc_p, uint64_t iterationNum){
  cfc = cfc_p;
  sharers = globals->allocBitSet(8 * sizeof(size_t));
  sharersOffset = iterationNum;
  globals->setBit(sharers, 0);
}

IterationInfo::~IterationInfo(){
  globals->deleteMem(cfc);
  globals->freeBitSet(sharers);
}

bool IterationInfo::doesCfcMatch(ControlFlowCompressor *cfc_p){
  return *cfc_p == *cfc;
}

void IterationInfo::markSharer(uint64_t iterationNum){
  assert(iterationNum >= sharersOffset);
  globals->setBit(sharers, iterationNum - sharersOffset);
}

bool IterationInfo::operator==(const IterationInfo& other){
  if(sharersOffset != other.sharersOffset)
    return false;
  else if(!xanBitSet_equal (sharers, other.sharers))
    return false;
  else
    return *cfc == *(other.cfc);
}

bool IterationInfo::operator!=(const IterationInfo& other){
  return !(*this == other);
}


/**
 * Invocation constructor.
 **/
InvocationInfo::InvocationInfo(void *inv, ExecTrace *t, uint64_t invocationNum)
  : invocation(inv), trace(t), sharersOffset(invocationNum), partiallyDumped(false)
{
  sharers = globals->allocBitSet(8 * sizeof(size_t));
  globals->setBit(sharers, 0);
}


/**
 * Get the invocation hash table.
 **/
void *
InvocationInfo::getInvocation(void)
{
  return invocation;
}

XanBitSet *
InvocationInfo::getSharers()
{
  return sharers;
}


/**
 * Mark a new sharer of the invocation.
 **/
void
InvocationInfo::markSharer(JITUINT64 invocationNum)
{
  assert(invocationNum >= sharersOffset);
  globals->setBit(sharers, invocationNum - sharersOffset);
}

JITBOOLEAN InvocationInfo::invocationsMatch(void *inv1, void *inv2){
  return trace->invocationsMatch(inv1, inv2);
}


/**
 * Invocation destructor.
 **/
InvocationInfo::~InvocationInfo()
{
  trace->clearInvocation(invocation);
  globals->freeBitSet(sharers);
}


/**
 * Execution trace constructor.
 **/
ExecTrace::ExecTrace()
  : numLatestInvocationGroups(32), startInvocation(0), numInvocations(0),
    nextInvocationToProcess(0)
{
  delayedInvocations = globals->allocHashTable(hashUint64AsPtr, matchUint64AsPtr);
  //allInvocationGroups = globals->allocHashTable(hashUint64AsPtr, matchUint64AsPtr);
  allInvocationGroups = globals->allocList();
  latestInvocationGroups = (InvocationInfo **)globals->allocMem(numLatestInvocationGroups * sizeof(InvocationInfo *));
  for (unsigned i = 0; i < numLatestInvocationGroups; ++i) {
    latestInvocationGroups[i] = NULL;
  }
}


/**
 * Execution trace destructor.
 **/
ExecTrace::~ExecTrace()
{
  clearTrace();
  globals->freeHashTable(delayedInvocations);
  globals->freeList(allInvocationGroups);
  globals->freeMem(latestInvocationGroups);
}


/**
 * Loop trace destructor.
 **/
LoopTrace::~LoopTrace()
{
  clearTrace();
}


/**
 * Call trace destructor.
 **/
CallTrace::~CallTrace()
{
  clearTrace();
}


/**
 * Clear an execution trace.
 **/
void
ExecTrace::clearTrace(void)
{
  assert(xanHashTable_elementsInside(delayedInvocations) == 0);
  for (unsigned i = 0; i < numLatestInvocationGroups; ++i) {
    latestInvocationGroups[i] = NULL;
  }
  XanListItem *groupItem = xanList_first(allInvocationGroups);
  while (groupItem) {
    InvocationInfo *group = (InvocationInfo *)groupItem->data;
    globals->deleteMem(group);
    //freeUint64AsPtr(groupItem->elementID);
    groupItem = groupItem->next;
  }
  globals->listEmptyOutList(allInvocationGroups);
}


/**
 * An invocation of a currently running structure.
 **/
RunningStruct::RunningStruct()
  : next(NULL)
{
  currCFC = globals->newMem<ControlFlowCompressor>(globals, COMPRESSION_WINDOW_SIZE);
  currInvocation = globals->allocHashTable();
}


/**
 * Currently running structure destructor.
 **/
RunningStruct::~RunningStruct()
{
  assert(xanHashTable_elementsInside(currInvocation) == 0);
  globals->freeHashTable(currInvocation);
  globals->deleteMem(currCFC);
}


/** 
 * An invocation of a currently running loop.
 **/
RunningLoop::RunningLoop()
{
  iterationInfos = globals->allocList();
}

/**
 * Currently running loop destructor.
 **/
RunningLoop::~RunningLoop()
{
  deleteIterationInfos();
}

void RunningLoop::deleteIterationInfos(){
  XanListItem *i = xanList_first(iterationInfos);
  while(i){
    globals->deleteMem((IterationInfo*)(i->data));
    i = i->next;
  }
  globals->freeList(iterationInfos);
}


/**
 * Get a new running call or loop structure from the pool.
 **/
template<class Running>
Running *
PassGlobals::fetchNewRunningStruct(Running **runningPool)
{
  Running *running;
  if (*runningPool == NULL) {
    running = newMem<Running>();
  } else {
    running = *runningPool;
    *runningPool = (Running *)running->next;
  }
  return running;
}


/**
 * Delete a pool of running structures.
 **/
template<class Running>
void
PassGlobals::deleteRunningPool(Running *runningPool)
{
  while (runningPool) {
    Running *running = runningPool;
    runningPool = (Running *)running->next;
    deleteMem(running);
  }
}


/**
 * Delete a hash table of traces.
 **/
template<class Trace>
void
PassGlobals::deleteTraceMap(XanHashTable *traces)
{
  XanHashTableItem *traceItem = xanHashTable_first(traces);
  while (traceItem) {
    Trace *trace = (Trace *)traceItem->element;
    deleteMem(trace);
    traceItem = xanHashTable_next(traces, traceItem);
  }
  freeHashTable(traces);
}


/**
 * Add a running call or loop structure back to the pool.
 **/
void
PassGlobals::freeRunningStruct(RunningStruct **runningPool, RunningStruct *running)
{
  running->next = *runningPool;
  *runningPool = running;
}

bool PassGlobals::recordCallTracesForInvocation(uint64_t invNum, bool attemptMatch){
  // Check if the current hash table of call traces is the same as for any previous invocation
  //XanListItem *prevInv = xanList_first(loopInvCallInfos);

  if(attemptMatch){
    for(auto callInfoIter = recentLoopInvCallInfos.begin(); callInfoIter != recentLoopInvCallInfos.end(); callInfoIter++){
      LoopInvocationCallInfo* callInfo = *callInfoIter;
      /* If a match is found, mark a new sharer */
      if(!callInfo->isPartiallyDumped() && callInfo->isEqual(callTraces)){
        callInfo->markSharer(invNum);
        /* Push this info to the front of the list */
        recentLoopInvCallInfos.erase(callInfoIter);
        recentLoopInvCallInfos.push_front(callInfo);
        return true;
      }
    }
    #if 0
    while(prevInv){
      LoopInvocationCallInfo* callInfo = (LoopInvocationCallInfo*)prevInv->data;

      /* If a match is found, mark a new sharer */
      if(!callInfo->isPartiallyDumped() && callInfo->isEqual(callTraces)){
        callInfo->markSharer(invNum);
        return true;
      }
      prevInv = prevInv->next;
    }
    #endif
  }
  
  /* No match was found, add the current call trace hash table and create a new one for the next invocation */
  LoopInvocationCallInfo* newLoopInvCallInfo = globals->newMem<LoopInvocationCallInfo>(invNum, callTraces, !attemptMatch);
  globals->listAppend(loopInvCallInfos, newLoopInvCallInfo);
  recentLoopInvCallInfos.push_front(newLoopInvCallInfo);
  if(recentLoopInvCallInfos.size() > MAX_RECENT_LOOP_INV_CALL_INFOS)
    recentLoopInvCallInfos.pop_back();
  return false;
}

LoopInvocationCallInfo::LoopInvocationCallInfo(uint64_t invNum, XanHashTable *callTraces_p, bool _partiallyDumped){
  sharers = globals->allocBitSet(8 * sizeof(size_t));
  sharersOffset = invNum;
  markSharer(invNum);
  callTraces = callTraces_p;
  partiallyDumped = _partiallyDumped;
}

void LoopInvocationCallInfo::deleteCallTraceTable(){
  XanHashTableItem* item = xanHashTable_first(callTraces);
  while(item){
    globals->deleteMem<CallTrace>((CallTrace*)(item->element));
    item = xanHashTable_next(callTraces, item);
  }
  globals->freeHashTable(callTraces);
}

LoopInvocationCallInfo::~LoopInvocationCallInfo(){
  globals->freeBitSet(sharers);
}

void LoopInvocationCallInfo::markSharer(uint64_t invNum){
  assert(invNum >= sharersOffset);
  globals->setBit(sharers, invNum - sharersOffset);
}

bool LoopInvocationCallInfo::isEqual(XanHashTable *newCallTraces){
  if(xanHashTable_elementsInside(newCallTraces) != xanHashTable_elementsInside(callTraces))
    return false;

  /* Cycle through each CallTrace and check if it is equal to the corresponding CallTrace */
  XanHashTableItem* item  = xanHashTable_first(newCallTraces);
  while(item){
    CallTrace* trace = (CallTrace*)xanHashTable_lookup(callTraces, item->elementID); // lookup the callID
    if(trace == NULL || *trace != *(CallTrace*)(item->element)) //check if the CallTrace for this callID matches
      return false;
    item = xanHashTable_next(newCallTraces, item);
  }
  return true;
}

XanHashTable* LoopInvocationCallInfo::getCallTraces(){
  return callTraces;
}

XanBitSet* LoopInvocationCallInfo::getSharers(){
  return sharers;
}

/* Return the number of call IDs with actual information in them */
unsigned int LoopInvocationCallInfo::getNumTraces(){
  uint64_t numTraces = 0;
  XanHashTableItem *traceItem = xanHashTable_first(callTraces);
  while (traceItem) {
    ExecTrace *trace = (ExecTrace *)traceItem->element;
    if (xanList_length(trace->allInvocationGroups) > 0)
      numTraces++;
    traceItem = xanHashTable_next(callTraces, traceItem);
  }
  return numTraces;
}

/**
 * Record all instructions that executed on the iteration that just finished.
 **/
void
RunningLoop::recordInstsSeenOnIteration(void)
{
  /* iterationNum is initialised to -1 so that the first call to IterationStart will 
   * set it to 0. Don't record anything before this first call to IterationStart.
  */
  if(iterationNum + 1 != 0){
    /* Check if it is the same as any previous iterations */
    //XanListItem* iterInfoItem = xanList_first(iterationInfos);
    bool matchFound = false;
    for(auto iterInfo = recentIterationMatches.begin(); iterInfo != recentIterationMatches.end(); iterInfo++){
      IterationInfo *iter = *iterInfo;
      if(iter->doesCfcMatch(currCFC)){
        iter->markSharer(iterationNum);
        currCFC->clear();
        matchFound = true;

        /* Move this match to the front of the queue */
        recentIterationMatches.erase(iterInfo);
        recentIterationMatches.push_front(iter);

        break;
      }
    }
    /*
    while(iterInfoItem){
      IterationInfo *iter = (IterationInfo *)iterInfoItem->data;
      if(iter->doesCfcMatch(currCFC)){
        iter->markSharer(iterationNum);
        currCFC->clear();
        matchFound = true;
        break;
      }
      iterInfoItem = iterInfoItem->next;
    }
    */

    /* No match with previous iterations, create a new one */
    if(!matchFound){
      IterationInfo* newIteration = globals->newMem<IterationInfo>(currCFC, iterationNum);
      globals->listAppend(iterationInfos, newIteration);
      currCFC = globals->newMem<ControlFlowCompressor>(globals, COMPRESSION_WINDOW_SIZE);
      /* Add the entry to the recently used iteration cache */
      recentIterationMatches.push_front(newIteration);
      if(recentIterationMatches.size() > MAX_RECENT_ITER_MATCHES)
        recentIterationMatches.pop_back();
    }
  }
}


/**
 * Shift the latest invocation groups up.
 **/
void
ExecTrace::shiftLatestInvocationGroups(unsigned top)
{
  for (unsigned i = top; i > 0; --i) {
    latestInvocationGroups[i] = latestInvocationGroups[i - 1];
  }
}


/**
 * Create a new invocation group.
 **/
void
ExecTrace::newInvocationGroup(void *invocation, JITUINT64 invocationNum, bool partiallyDumped)
{
  InvocationInfo *info = globals->newMem<InvocationInfo>(invocation, this, invocationNum);
  if(partiallyDumped)
    info->setPartiallyDumped();
  globals->listAppend(allInvocationGroups, info);
  //globals->hashTableInsert(allInvocationGroups, createUint64AsPtr(invocationNum), info);
  if (latestInvocationGroups[0] == NULL) {
    startInvocation = invocationNum;
  } else {
    shiftLatestInvocationGroups(numLatestInvocationGroups - 1);
  }
  latestInvocationGroups[0] = info;
}


/**
 * Check whether two hash tables to instruction dynamic instances match.
 **/
JITBOOLEAN
LoopTrace::dynamicInstancesMatch(void *i1, void *i2)
{
  XanHashTableItem *instanceItem;

  /* Cast to the loop trace instance type. */
  XanHashTable *instDynInstances = (XanHashTable *)i1;
  XanHashTable *groupDynInstances = (XanHashTable *)i2;

  /* Easy checks. */
  if (xanHashTable_elementsInside(instDynInstances) != xanHashTable_elementsInside(groupDynInstances)) {
    return JITFALSE;
  }

  /* Go through each iteration. */
  instanceItem = xanHashTable_first(instDynInstances);
  while (instanceItem) {
    XanHashTableItem *gInstanceItem = xanHashTable_lookupItem(groupDynInstances, instanceItem->elementID);
    if (!gInstanceItem || getUint64AsPtr(gInstanceItem->element) != getUint64AsPtr(instanceItem->element)) {
      return JITFALSE;
    }
    instanceItem = xanHashTable_next(instDynInstances, instanceItem);
  }

  /* Checks passed. */
  return JITTRUE;
}


/**
 * Check whether two 64 bit integer pointers point to the same number.
 **/
JITBOOLEAN
CallTrace::dynamicInstancesMatch(void *i1, void *i2)
{
  return getUint64AsPtr(i1) == getUint64AsPtr(i2);
}

ostream& operator<<(ostream& os, const CallTrace& call){
  XanListItem* item = xanList_first(call.allInvocationGroups);
  while(item){
    InvocationInfo* info = (InvocationInfo*)item->data;
    os << *(ControlFlowCompressor*)info->getInvocation() << endl;
    item = item->next;
  }
  return os;
}


/**
 * Compare two invocations to determine whether they match.
 **/
JITBOOLEAN
LoopTrace::invocationsMatch(void *inv1, void *inv2)
{
  XanList * invoc1 = (XanList*)inv1;
  XanList * invoc2 = (XanList*)inv2;
  if(xanList_length(invoc1) != xanList_length(invoc2))
    return false;
  else{
    XanListItem * invoc1Item = xanList_first(invoc1);
    XanListItem * invoc2Item = xanList_first(invoc2);
    while(invoc1Item){
      if(*(IterationInfo*)invoc1Item->data != *(IterationInfo*)invoc2Item->data)
        return false;
      invoc1Item = invoc1Item->next;
      invoc2Item = invoc2Item->next;
    }
  }
  return true;
}

JITBOOLEAN
CallTrace::invocationsMatch(void *inv1, void *inv2)
{
  return (*(ControlFlowCompressor*)inv1 == *(ControlFlowCompressor*)inv2);
}


bool CallTrace::operator==(const CallTrace& other){
  if(xanList_length(allInvocationGroups) != xanList_length(other.allInvocationGroups))
    return false;

  /* Compare each invocation (list item), presumably if they are equal the lists would have been filled in the same order */
  XanListItem *item1 = xanList_first(allInvocationGroups);
  XanListItem *item2 = xanList_first(other.allInvocationGroups);
  while(item1){
    InvocationInfo *info1 = (InvocationInfo*) item1->data;
    InvocationInfo *info2 = (InvocationInfo*) item2->data;
    if(!info1->invocationsMatch(info1->getInvocation(), info2->getInvocation()) 
        || !xanBitSet_equal(info1->getSharers(), info2->getSharers()))
      return false;
    item1 = item1->next;
    item2 = item2->next;
  }

#if 0
  /* Compare each invocation in turn */
  XanHashTableItem * item = xanList_first(allInvocationGroups);
  while (item) {
    /* Lookup this invocation in other */
    InvocationInfo *otherInfo = (InvocationInfo*)xanHashTable_lookup(other.allInvocationGroups, item->elementID);

    if(otherInfo == NULL)
      return false;

    InvocationInfo *info = (InvocationInfo *)item->element;
    if(!info->invocationsMatch(info->getInvocation(), otherInfo->getInvocation()) || !xanBitSet_equal(info->getSharers(), otherInfo->getSharers()))
      return false;

    /* Next invocation group. */
    item = xanHashTable_next(allInvocationGroups, item);
  }
#endif

  return true;
}

bool CallTrace::operator!=(const CallTrace& other){
  return !(*this == other);
}



/**
 * Process an invocation to record instructions executed.  Returns JITTRUE 
 * if the invocation starts a new group, and therefore the hash table cannot be
 * reused.
 **/
JITBOOLEAN
ExecTrace::processInvocation(void *invocation, JITUINT64 invocationNum, bool attemptMatch)
{
  /* First invocation processed. */
  if (latestInvocationGroups[0] == NULL) {
    newInvocationGroup(invocation, invocationNum, !attemptMatch);
    return JITTRUE;
  }

  if(attemptMatch){
    /* Check all latest groups. */
    for (unsigned i = 0; i < numLatestInvocationGroups; ++i) {

      /* No more groups to process. */
      if (latestInvocationGroups[i] == NULL) {
        break;
      }

      /* Check all instructions, iterations and dynamic instances match. */
      if (!latestInvocationGroups[i]->isPartiallyDumped() && 
          invocationsMatch(invocation, latestInvocationGroups[i]->getInvocation())) {
        latestInvocationGroups[i]->markSharer(invocationNum);
        InvocationInfo *info = latestInvocationGroups[i];
        shiftLatestInvocationGroups(i);
        latestInvocationGroups[0] = info;
        return JITFALSE;
      }
    }
  }

  /* No match found in prior invocations. */
  newInvocationGroup(invocation, invocationNum, !attemptMatch);
  return JITTRUE;
}


/**
 * Free an invocation element.
 **/
void
LoopTrace::freeInvocation(void *invocation)
{
  XanList *invoc = (XanList*)invocation;
  XanListItem *invocItem = xanList_first(invoc);
  while(invocItem){
    globals->deleteMem((IterationInfo*)invocItem->data);
    invocItem = invocItem->next;
  }
  globals->freeList(invoc);
}


/**
 * Free an invocation element.
 **/
void
CallTrace::freeInvocation(void *invocation) {
  globals->deleteMem((ControlFlowCompressor*)invocation);
}


/**
 * Clear an invocation hash table.
 **/
void
ExecTrace::clearInvocation(void *invocation) {
  freeInvocation(invocation);
}

void RunningCall::recordInstsSeenOnInvocation(bool attemptMatch){
  if(trace->processInvocation(currCFC, invocationNum, attemptMatch))
    currCFC = globals->newMem<ControlFlowCompressor>(globals, COMPRESSION_WINDOW_SIZE);
  else
    currCFC->clear();
  trace->nextInvocationToProcess += 1;
}

void RunningLoop::recordInstsSeenOnInvocation(bool attemptMatch){
  if(!(trace->processInvocation(iterationInfos, invocationNum, attemptMatch)))
    deleteIterationInfos();
  recentIterationMatches.clear();
  iterationInfos = globals->allocList();
  trace->nextInvocationToProcess += 1;
}

#if 0
/**
 * Process delayed invocations in this trace.
 **/
void
ExecTrace::processDelayedInvocations()
{
  XanHashTableItem *delayedItem;
  while ((delayedItem = xanHashTable_lookupItem(delayedInvocations, keyUint64AsPtr(&nextInvocationToProcess))) != NULL) {
    XanHashTable *delayed = (XanHashTable *)delayedItem->element;
    if (!processInvocation(delayed, nextInvocationToProcess, true)) {
      clearInvocation(delayed);
      globals->freeHashTable(delayed);
    }
    freeUint64AsPtr(delayedItem->elementID);
    globals->hashTableRemoveItem(delayedInvocations, delayedItem);
    nextInvocationToProcess += 1;
  }
}
#endif


/**
 * Record an instruction as seen in this loop.
 **/
void
RunningLoop::seenInstruction(JITNINT instID)
{
  currCFC->insertSymbol(instID);
}


/**
 * Record an instruction as seen in this call.
 **/
void
RunningCall::seenInstruction(JITNINT instID)
{
  currCFC->insertSymbol(instID);
}


/**
 * Do all that's need to be done to finish an invocation.
 **/
void
RunningLoop::finishInvocation(bool attemptMatch)
{
  recordInstsSeenOnIteration();
  recordInstsSeenOnInvocation(attemptMatch);
}


/**
 * Do all that's need to be done to finish an invocation.
 **/
void
RunningCall::finishInvocation(bool attemptMatch)
{
  recordInstsSeenOnInvocation(attemptMatch);
}


/**
 * Write to a compressed output file and check for errors.
 **/
void
writeCompressedFile(BZFILE *compressedFile, char *buf)
{
  JITINT32 errorCode;
  BZ2_bzWrite(&errorCode, compressedFile, buf, strlen(buf));
  if (errorCode != BZ_OK) {
    cerr << "Failed writing compressed file with error code " << errorCode << endl;
    perror(NULL);
    abort();
  }
}


/**
 * Write to an uncompressed output file and check for errors.
 **/
void
writeFile(FILE *file, char *buf)
{
  int bytes = fprintf(file, "%s", buf);
  if (bytes < 0) {
    cerr << "Failed writing file with error code " << errno << endl;
    perror(NULL);
    abort();
  }
}

void writeRangeToCompressedFile(XanBitSet *sharers, uint64_t sharersOffset, BZFILE *compressedFile){
  char buf[DIM_BUF];

  /* Start off the invocation numbers. */
  snprintf(buf, DIM_BUF, "{");
  writeCompressedFile(compressedFile, buf);

  /* Write the start and end invocations. */
  JITINT64 startInv = -1;
  JITBOOLEAN first = JITTRUE;
  JITUINT64 numSharers = xanBitSet_length(sharers);
  while ((startInv = xanBitSet_getFirstBitSetInRange(sharers, startInv + 1, numSharers)) != -1) {
    JITINT64 endInv = xanBitSet_getFirstBitUnsetInRange(sharers, startInv, numSharers);
    if (endInv == -1) {
      endInv = numSharers - 1;
    } else {
      endInv -= 1;
    }
    assert(endInv >= startInv);

    /* Correct for sharer offset */
    uint64_t startInvocation = sharersOffset + startInv;
    uint64_t endInvocation = sharersOffset + endInv;


    /* Write this invocation range. */
    if (first) {
      if (startInvocation == endInvocation) {
        snprintf(buf, DIM_BUF, "%" PRIu64, startInvocation);
      } else {
        snprintf(buf, DIM_BUF, "%" PRIu64 "-%" PRIu64, startInvocation, endInvocation);
      }
      first = JITFALSE;
    } else {
      if (startInvocation == endInvocation) {
        snprintf(buf, DIM_BUF, ",%" PRIu64, startInvocation);
      } else {
        snprintf(buf, DIM_BUF, ",%" PRIu64 "-%" PRIu64, startInvocation, endInvocation);
      }
    }
    writeCompressedFile(compressedFile, buf);

    /* Skip to the end of this range. */
    startInv = endInv + 1;
  }

  /* Finish the invocation ranges. */
  snprintf(buf, DIM_BUF, "} ");
  writeCompressedFile(compressedFile, buf);
}


/**
 * Write a loop invocation to a file.
 **/
void
LoopTrace::writeInvocationToFile(void *invocation, BZFILE *compressedFile)
{
  XanList *invocList = (XanList*)invocation;

  /* Write the parsing hint (number of iteration groups ) */
  char buf[DIM_BUF];
  snprintf(buf, DIM_BUF, "%d\n", xanList_length(invocList));
  writeCompressedFile(compressedFile, buf);

  /* Write each iteration range and compressed trace */
  XanListItem *invocListItem = xanList_first(invocList);
  while(invocListItem){
    IterationInfo *iterInfo = (IterationInfo*)(invocListItem->data);

    writeRangeToCompressedFile(iterInfo->getSharers(), iterInfo->getSharersOffset(), compressedFile);
    std::ostringstream stream;
    stream << *iterInfo->getCfc();
    std::string str =  stream.str();
    char* chr = const_cast<char *>(str.c_str());
    writeCompressedFile(compressedFile, chr);
    writeCompressedFile(compressedFile, (char *)"\n");

    invocListItem = invocListItem->next;
  }
}


/**
 * Write a call invocation to a file.
 **/
void
CallTrace::writeInvocationToFile(void *invocation, BZFILE *compressedFile)
{
  std::ostringstream stream;
  stream << *(ControlFlowCompressor*)invocation;
  std::string str =  stream.str();
  char* chr = const_cast<char *>(str.c_str());
  writeCompressedFile(compressedFile, chr);
}


/**
 * Write an invocation group to a file.
 **/
void
InvocationInfo::writeInvocationToFile(BZFILE *compressedFile)
{
  /* Write the invocation ranges */
  writeRangeToCompressedFile(sharers, sharersOffset, compressedFile);

  /* Write the actual trace. */
  trace->writeInvocationToFile(invocation, compressedFile);
}

/**
 * Set the partiallyDumped bool to indicate that this is a continuation 
 * of a previous dump.
 * This is useful because we want to avoid attempting to match full
 * dumps with partial dumps as this makes parsing difficult
 **/
void 
InvocationInfo::setPartiallyDumped(){ 
  partiallyDumped = true;
}

bool
InvocationInfo::isPartiallyDumped(){ 
  return partiallyDumped;
}


/**
 * Write an execution trace to a file.
 **/
void
ExecTrace::writeTraceToFile(uintptr_t id, BZFILE *compressedFile)
{
  char buf[DIM_BUF];
  if (xanList_length(allInvocationGroups) > 0) {

    /* Print this trace ID. */
    snprintf(buf, DIM_BUF, "%" PRIuPTR " %u\n", id, xanList_length(allInvocationGroups));
    writeCompressedFile(compressedFile, buf);

    /* Work through all invocations. */
    XanListItem *groupItem = xanList_first(allInvocationGroups);
    while (groupItem) {
      InvocationInfo *info = (InvocationInfo *)groupItem->data;

      /* Write this invocation. */
      info->writeInvocationToFile(compressedFile);

      /* Finish the line. */
      snprintf(buf, DIM_BUF, "\n");
      writeCompressedFile(compressedFile, buf);

      /* For loop traces, indicate if this invocation is complete */
      if(!isCallTrace() && !(groupItem == xanList_last(allInvocationGroups)))
        globals->writeDumpCompleteSymbol(compressedFile);

      /* Next invocation group. */
      groupItem = groupItem->next;
    }

    /* Finish this trace. */
    snprintf(buf, DIM_BUF, "\n");
    writeCompressedFile(compressedFile, buf);
  }
}


/**
 * Write a table of traces to a file.
 **/
void
PassGlobals::writeTracesToFile(BZFILE *compressedFile, XanHashTable *traces)
{
  XanHashTableItem *traceItem;
  JITINT32 traceNum = 0;

  /* Write each loop. */
  traceItem = xanHashTable_first(traces);
  while (traceItem) {
    ExecTrace *trace = (ExecTrace *)traceItem->element;
    trace->writeTraceToFile(ptrToInt(traceItem->elementID), compressedFile);
    traceNum += 1;
    traceItem = xanHashTable_next(traces, traceItem);
  }
}

void
PassGlobals::writeDumpIncompleteSymbol(BZFILE *compressedFile)
{
  char buf[20];
  strcpy(buf, "\nINCOMPLETE\n");
  writeCompressedFile(compressedFile, buf);
}

void
PassGlobals::writeDumpCompleteSymbol(BZFILE *compressedFile)
{
  char buf[20];
  strcpy(buf, "\nCOMPLETE\n");
  writeCompressedFile(compressedFile, buf);
}

void 
PassGlobals::openCompressedFiles(){
  /* Open the output file. */
  loopOutputFile = fopen((outputDirectory + "/loop_trace.txt.bz2").c_str(), "w");
  if (!loopOutputFile) {
    abort();
  }
  loopCompressedFile = BZ2_bzWriteOpen(NULL, loopOutputFile, 9, 0, 250);
  if (!loopCompressedFile) {
    abort();
  }
  callOutputFile = fopen((outputDirectory + "/call_trace.txt.bz2").c_str(), "w");
  if (!callOutputFile) {
    abort();
  }
  callCompressedFile = BZ2_bzWriteOpen(NULL, callOutputFile, 9, 0, 250);
  if (!callCompressedFile) {
    abort();
  }
}

bool
PassGlobals::compressedFilesOpen(){
  return (loopCompressedFile != NULL);
}

void 
PassGlobals::closeCompressedFiles(){
  /* Close output file. */
  BZ2_bzWriteClose(NULL, loopCompressedFile, 0, NULL, NULL);
  fclose(loopOutputFile);
  BZ2_bzWriteClose(NULL, callCompressedFile, 0, NULL, NULL);
  fclose(callOutputFile);
}


/**
 * Write out the instructions corresponding to call invocations and loop
 * invocations and iterations.
 **/
void
PassGlobals::writeTraces(string id)
{
  if(!compressedFilesOpen()){
    openCompressedFiles();
  }
  if (xanHashTable_elementsInside(loopTraces) > 0) {
    writeTracesToFile(loopCompressedFile, loopTraces);
    if(loopRunning())
      writeDumpIncompleteSymbol(loopCompressedFile);
    else
      writeDumpCompleteSymbol(loopCompressedFile);
  }
  if (xanList_length(loopInvCallInfos) > 0) {

    char buf[DIM_BUF];

    /* Write the number of invocation entries */
    snprintf(buf, DIM_BUF, "%i\n", xanList_length(loopInvCallInfos));
    writeCompressedFile(callCompressedFile, buf);

    /* Write each invocation entry */
    XanListItem *item = xanList_first(loopInvCallInfos);
    while(item){
      LoopInvocationCallInfo* callInfo = (LoopInvocationCallInfo*)item->data;

      /* Write the loop invocation range pattern */
      writeRangeToCompressedFile(callInfo->getSharers(), callInfo->getSharersOffset(), callCompressedFile);

      /* Write the number of call IDs */
      snprintf(buf, DIM_BUF, " %u\n", callInfo->getNumTraces());//xanHashTable_elementsInside(callInfo->getCallTraces()));
      writeCompressedFile(callCompressedFile, buf);

      /* Write the call traces */
      writeTracesToFile(callCompressedFile, callInfo->getCallTraces());
      
      /* Indicate if the invocation is complete */
      if(item != xanList_last(loopInvCallInfos))
        writeDumpCompleteSymbol(callCompressedFile);
        
      item = item->next;
    }

    if(loopRunning())
      writeDumpIncompleteSymbol(callCompressedFile);
    else
      writeDumpCompleteSymbol(callCompressedFile);
    //closeCompressedFile();
  }
}

/* This assumes there is at most one loop in the stack */
uint64_t PassGlobals::getCurrentLoopInvocationNumber(){
  XanList *runningStructs = globals->stackToList(runningStack);
  XanListItem *runningItem = xanList_first(runningStructs);
  while (runningItem) {
    RunningStruct *running = (RunningStruct *)runningItem->data;
    if (running) {
      if (!running->isCall()){
        globals->freeList(runningStructs);
        return running->invocationNum;
      }
    }
    runningItem = runningItem->next;
  }
  cerr << "No loop found in stack?!\n";
  abort();
}


void PassGlobals::dumpTraces(){
  //cerr << "ABORT: Intermediate dump not currently implemented for loop trace\n";
  //abort();

  /* Save any information within the traces that could be altered. */
  //XanHashTable *nextInvocations = globals->allocHashTable(hashUint64AsPtr, matchUint64AsPtr);
  XanHashTable *nextInvocations = globals->allocHashTable();
  XanHashTableItem *traceItem = xanHashTable_first(loopTraces);
  while (traceItem) {
    ExecTrace *trace = (ExecTrace *)traceItem->element;
    globals->hashTableInsert(nextInvocations, trace, createUint64AsPtr(trace->nextInvocationToProcess));
    traceItem = xanHashTable_next(loopTraces, traceItem);
  }
  traceItem = xanHashTable_first(callTraces);
  while (traceItem) {
    ExecTrace *trace = (ExecTrace *)traceItem->element;
    globals->hashTableInsert(nextInvocations, trace, createUint64AsPtr(trace->nextInvocationToProcess));
    traceItem = xanHashTable_next(callTraces, traceItem);
  }

  /* Write all current data into the traces. */
  XanList *runningStructs = globals->stackToList(runningStack);
  XanListItem *runningItem = xanList_first(runningStructs);
  while (runningItem) {
    RunningStruct *running = (RunningStruct *)runningItem->data;
    if (running) {
      running->partiallyDumped = true;
      running->finishInvocation(false);
    }
    runningItem = runningItem->next;
  }
  globals->freeList(runningStructs);

  /* Save the call traces that have occurred in this loop invocation so far, if
   * matchFound is true, ownership of callTraces has not been transfered */
  /* I am assuming that only one loop and call can be running at a given time (!) */ 
  if(loopRunning()){
    uint64_t loopInvocationNum = globals->getCurrentLoopInvocationNumber();
    bool matchFound = globals->recordCallTracesForInvocation(loopInvocationNum, false);
    assert(!matchFound);
  }

  /* Write the traces. */
  writeTraces(to_string(traceDumpID));
  traceDumpID += 1;

  /* If there was a call running, we need to reallocate the trace for it */
  #if 0
  runningStructs = globals->stackToList(runningStack);
  runningItem = xanList_first(runningStructs);
  while (runningItem) {
    RunningStruct *running = (RunningStruct *)runningItem->data;
    if (running && running->isCall()){
      CallTrace* trace = globals->newMem<CallTrace>();
      trace->numInvocations = numInvocations;
      globals->hashTableInsert(globals->callTraces, callID, trace);
      running->trace = trace;
    }
    runningItem = runningItem->next;
  }
  globals->freeList(runningStructs);
  #endif

  /* Clear traces and restore trace information. */
  traceItem = xanHashTable_first(nextInvocations);
  while (traceItem) {
    ExecTrace *trace = (ExecTrace *)traceItem->elementID;
    trace->clearTrace();
    trace->nextInvocationToProcess = getUint64AsPtr(traceItem->element);
    freeUint64AsPtr(traceItem->element);
    traceItem = xanHashTable_next(nextInvocations, traceItem);
  }
  globals->freeHashTable(nextInvocations);

  /* Clear out the call trace loop invocations structure */
  //globals->deleteAllLoopInvCallTraces();
  globals->clearAllLoopInvCallTraces(loopRunning());
  globals->recentLoopInvCallInfos.clear();
  freeList(loopInvCallInfos);
  loopInvCallInfos = globals->allocList();

  globals->waitingForInvocCompletion = true;
}

/**
 * Make a dump of all current traces if too much memory has been allocated.
 **/
void
PassGlobals::checkDumpTraces(void)
{
  if(xanStack_getSize(globals->runningStack) < 2)
    return;
  if (memUsed > dumpTraceMemUsage) {
    dumpTraces();
  }
}

bool PassGlobals::loopRunning(){
  XanList *runningStructs = globals->stackToList(runningStack);
  XanListItem *runningItem = xanList_first(runningStructs);
  while (runningItem) {
    RunningStruct *running = (RunningStruct *)runningItem->data;
    if (running && !running->isCall()){
      globals->freeList(runningStructs);
      return true;
    }
    runningItem = runningItem->next;
  }
  globals->freeList(runningStructs);
  return false;
}

bool PassGlobals::callRunning(){
  XanList *runningStructs = globals->stackToList(runningStack);
  XanListItem *runningItem = xanList_first(runningStructs);
  while (runningItem) {
    RunningStruct *running = (RunningStruct *)runningItem->data;
    if (running && running->isCall()){
      globals->freeList(runningStructs);
      return true;
    }
    runningItem = runningItem->next;
  }
  globals->freeList(runningStructs);
  return false;
}


/**
 * Start a loop (i.e. a new invocation).
 **/
void
CAM_profileLoopInvocationStart(JITNINT loopID)
{
  if(timeoutCounter->recordOperation())
    return;

  RunningLoop *running;
  LoopTrace *trace;
  PDEBUG("Start loop %d (stack: %d)\n", loopID, xanStack_getSize(globals->runningStack));
  if(xanStack_getSize(globals->runningStack) > 1){
    cerr << "Starting loop when there is already something in the runningStack\n";
    while(xanStack_getSize(globals->runningStack) > 1){
      running = (RunningLoop *)globals->stackPop(globals->runningStack);
      running->displaySummary();
    }
    abort();
  }


  /* Start a new loop. */
  running = globals->fetchNewRunningLoop();
  trace = (LoopTrace *)xanHashTable_lookup(globals->loopTraces, intToPtr(loopID));
  if (!trace) {
    trace = globals->newMem<LoopTrace>();
    globals->hashTableInsert(globals->loopTraces, intToPtr(loopID), trace);
  }
  running->trace = trace;;
  running->invocationNum = trace->numInvocations;
  running->iterationNum = -1;
  running->partiallyDumped = false;
  trace->numInvocations += 1;

  /* Push this onto the stack. */
  globals->stackPush(globals->runningStack, running);
  globals->checkDumpTraces();
}


/**
 * Finish an invocation of a loop.
 **/
void
CAM_profileLoopInvocationEnd(void)
{
  if(timeoutCounter->recordOperation())
    return;

  PDEBUG("End loop (stack: %d)\n", xanStack_getSize(globals->runningStack));
  RunningLoop *running = (RunningLoop *)globals->stackPop(globals->runningStack);
  running->finishInvocation(!(running->partiallyDumped));
  globals->freeRunningLoop(running);

  /* Add all call traces for this invocation to the list of LoopInvocationCallInfos */
  if(globals->recordCallTracesForInvocation(running->invocationNum, !globals->waitingForInvocCompletion))
    globals->deleteTraceMap<CallTrace>(globals->callTraces);
  globals->callTraces = globals->allocHashTable();
  globals->waitingForInvocCompletion = false;

  globals->checkDumpTraces();
}


/**
 * Start a new loop iteration.
 **/
void
CAM_profileLoopIterationStart(void)
{
  if(timeoutCounter->recordOperation())
    return;

  PDEBUG("Start iteration\n");
  RunningLoop *running = (RunningLoop *)xanStack_top(globals->runningStack);
  running->recordInstsSeenOnIteration();
  running->iterationNum += 1;
  globals->checkDumpTraces();
}


/**
 * Record that an instruction has been seen.
 **/
void
CAM_profileLoopSeenInstruction(JITNINT instID)
{

  RunningStruct *running = (RunningStruct *)xanStack_top(globals->runningStack);
  if (running) {
    if(timeoutCounter->recordOperation())
      return;
    //PDEBUG("Seen instruction %u\n", instID);
    running->seenInstruction(instID);
    globals->checkDumpTraces();
  }
}


/**
 * Start an invocation of a call.
 **/
void
CAM_profileCallInvocationStart(JITNINT instID)
{

  RunningStruct *running = (RunningStruct *)xanStack_top(globals->runningStack);

  /* Must be running a loop. */
  if (running) {
    if(timeoutCounter->recordOperation())
      return;
    CallTrace *trace;
    PDEBUG("Start call %d (stack: %d)\n", instID, xanStack_getSize(globals->runningStack));

    /* Start a new call. */
    running = globals->fetchNewRunningCall();
    trace = (CallTrace *)xanHashTable_lookup(globals->callTraces, intToPtr(instID));
    if (!trace) {
      trace = globals->newMem<CallTrace>();
      globals->hashTableInsert(globals->callTraces, intToPtr(instID), trace);
    }
    running->trace = trace;
    running->invocationNum = trace->numInvocations;
    running->partiallyDumped = false;
    trace->numInvocations += 1;

    /* Push this onto the stack. */
    globals->stackPush(globals->runningStack, running);
    globals->checkDumpTraces();
  }
}


/**
 * End an invocation of a call.
 **/
void
CAM_profileCallInvocationEnd(void)
{

  RunningCall *running = (RunningCall *)xanStack_top(globals->runningStack);
  if (running) {
    if(timeoutCounter->recordOperation())
      return;
    PDEBUG("End call (stack: %d)\n", xanStack_getSize(globals->runningStack));
    globals->stackPop(globals->runningStack);
    running->finishInvocation(!(running->partiallyDumped));
    globals->freeRunningCall(running);
    globals->checkDumpTraces();
  }
}

void CAM_forceLoopTraceDump(){
  globals->dumpTraces();
}

/**
 * Initialisation.
 **/
void
loop_trace_init(void)
{
  globals = new PassGlobals();
  timeoutCounter = new TimeoutCounter();
}


/**
 * Shut down.
 **/
void
loop_trace_shutdown(void)
{
  
  /* If the library was actually used, dump output */
  if(timeoutCounter->getNumOperations() > 0){
    if(!globals->compressedFilesOpen()){
      globals->openCompressedFiles();
    }

    if (globals->traceDumpID > 0) {
      globals->writeTraces(to_string(globals->traceDumpID));
    } else {
      globals->writeTraces("0");
    }

    globals->closeCompressedFiles();

    if(!timeoutCounter->isTimedOut() && xanStack_getSize (globals->runningStack) > 1){
      cerr << "Running stack is not empty (" << xanStack_getSize(globals->runningStack) << "), calls to libcam API were inconsistent\n";
      abort();
    }

    timeoutCounter->dumpStats(globals->getOutputDirectory());
  }

  /* Cleanup */
  delete timeoutCounter;
  delete globals;
  globals = NULL;
}
