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

#ifndef CAM_LOOP_TRACE_H
#define CAM_LOOP_TRACE_H

#include <bzlib.h>
#include <xanlib.h>
#include "ControlFlowCompressor.h"


/* Forward declaration. */
class InvocationInfo;


/**
 * Information held about which instructions have been executed on each call
 * invocation or loop invocation and iteration.
 **/
class ExecTrace
{
public:
  JITUINT32 numLatestInvocationGroups;
  JITUINT64 startInvocation;
  JITUINT64 numInvocations;
  JITUINT64 nextInvocationToProcess;
  XanHashTable *delayedInvocations;
  XanList *allInvocationGroups;
  InvocationInfo **latestInvocationGroups;

  ExecTrace();

  virtual ~ExecTrace();

  /* Create a new invocation group. */
  virtual void newInvocationGroup(void *invocation, JITUINT64 invocationNum, bool partiallyDumped);

  ///* Process delayed invocations. */
  //virtual void processDelayedInvocations(void);

  /* Shift the latest invocation groups. */
  virtual void shiftLatestInvocationGroups(unsigned top);

  /* Clear out a trace. */
  virtual void clearTrace(void);

  /* Clear an invocation hash table. */
  virtual void clearInvocation(void *invocation);

  /* Free a single invocation element. */
  virtual void freeInvocation(void *invocation) = 0;

  /* Check whether two dynamic instances match. */
  virtual JITBOOLEAN dynamicInstancesMatch(void *i1, void *i2) = 0;

  /* Check whether two invocations match. */
  virtual JITBOOLEAN invocationsMatch(void *inv1, void *inv2) = 0;

  /* Process an invocation. */
  virtual JITBOOLEAN processInvocation(void *invocation, JITUINT64 invocationNum, bool attemptMatch);

  /* Write in invocation to a file. */
  virtual void writeInvocationToFile(void *invocation, BZFILE *compressedFile) = 0;

  /* Write the trace to a file. */
  virtual void writeTraceToFile(uintptr_t id, BZFILE *compressedFile);

  virtual bool isCallTrace() = 0;
};


/**
 * Information held about which instructions have been executed on each loop
 * invocation and iteration.
 **/
class LoopTrace : public ExecTrace
{
public:
  virtual ~LoopTrace();

  /* Get list of all instruction IDs recorded in this loop */
  XanList *getInstructionIDs();

  /* Free a single invocation element. */
  void freeInvocation(void *invocation);

  /* Check whether two dynamic instances match. */
  JITBOOLEAN dynamicInstancesMatch(void *i1, void *i2);

  /* Check whether two invocations match. */
  JITBOOLEAN invocationsMatch(void *inv1, void *inv2);

  /* Write in invocation to a file. */
  void writeInvocationToFile(void *invocation, BZFILE *compressedFile);

  bool isCallTrace(){ return false; }
};


/**
 * Information held about which instructions have been executed on each call
 * invocation.
 **/
class CallTrace : public ExecTrace
{
public:
  virtual ~CallTrace();

  /* Get list of all instruction IDs recorded in this loop */
  XanList *getInstructionIDs();

  /* Free a single invocation element. */
  void freeInvocation(void *invocation);

  /* Check whether two dynamic instances match. */
  JITBOOLEAN dynamicInstancesMatch(void *i1, void *i2);

  /* Check whether two invocations match. */
  JITBOOLEAN invocationsMatch(void *inv1, void *inv2);

  /* Write in invocation to a file. */
  void writeInvocationToFile(void *invocation, BZFILE *compressedFile);

  bool isCallTrace(){ return true; }

  bool operator==(const CallTrace& other);
  bool operator!=(const CallTrace& other);
  friend ostream& operator<<(ostream& os, const CallTrace& call);
};

/*
 * Information for all calls within a single invocation of the loop 
 */
class LoopInvocationCallInfo{
  XanHashTable *callTraces;      /**< Map from call inst ID to a call trace. */
  XanBitSet *sharers;
  uint64_t sharersOffset;
  bool partiallyDumped;

public:
  LoopInvocationCallInfo(uint64_t invNum, XanHashTable *callTraces_p, bool _partiallyDumped);
  ~LoopInvocationCallInfo();
  void deleteCallTraceTable();
  void markSharer(uint64_t invNum);
  bool isEqual(XanHashTable *newCallTraces);
  XanHashTable* getCallTraces();
  XanBitSet* getSharers();
  unsigned int getNumTraces();
  uint64_t getSharersOffset() { return sharersOffset; }
  bool isPartiallyDumped() { return partiallyDumped; }
};


/** 
 * Information for a single iteration within a loop 
 */
class IterationInfo{
  ControlFlowCompressor *cfc;
  XanBitSet *sharers;
  uint64_t sharersOffset;

public: 
  IterationInfo(ControlFlowCompressor *cfc, uint64_t iterationNum);
  ~IterationInfo();

  bool doesCfcMatch(ControlFlowCompressor *cfc);

  void markSharer(uint64_t iterationNum);

  XanBitSet *getSharers(){ return sharers; }
  uint64_t getSharersOffset() { return sharersOffset; }

  ControlFlowCompressor *getCfc() { return cfc; }

  bool operator==(const IterationInfo& other);
  bool operator!=(const IterationInfo& other);
};


/**
 * Information about a single invocation of a call or loop along with the
 * invocation numbers that share this data.
 **/
class InvocationInfo
{
private:
  void *invocation;
  ExecTrace *trace;
  XanBitSet *sharers;
  uint64_t sharersOffset;
  bool partiallyDumped;

public:
  InvocationInfo(void *inv, ExecTrace *t, uint64_t invocationNum);

  ~InvocationInfo();

  /* Clear out this invocation. */
  void clearInvocation(void);

  /* Get the invocation. */
  void *getInvocation(void);

  /* Check whether two invocations match. */
  JITBOOLEAN invocationsMatch(void *inv1, void *inv2);

  /* Return the sharers bit set */
  XanBitSet *getSharers();
  uint64_t getSharersOffset() { return sharersOffset; }

  /* Mark a new sharer. */
  void markSharer(JITUINT64 invocationNum);

  /* Write this invocation to a file. */
  void writeInvocationToFile(BZFILE *compressedFile);

  void setPartiallyDumped();
  bool isPartiallyDumped();
};


/* Initialisation. */
void loop_trace_init(void);

/* Shut down. */
void loop_trace_shutdown(void);

#endif /* CAM_LOOP_TRACE_H */
