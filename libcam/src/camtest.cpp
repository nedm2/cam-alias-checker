
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

#include <iostream>
#include "cam.h"
#include "memory_allocator.hh"
#include <assert.h>
#include "loop_trace.hh"
#include "ControlFlowCompressor.h"
#include "parser_wrappers.h"
#include "MemoryTraceStreamer.h"
#include "LoopTraceStreamer.h"
#include "CallTraceStreamer.h"
#include "CallTrace.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace std;

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
    JITUINT64 *u = (JITUINT64 *)malloc(sizeof(JITUINT64));
    *u = uint;
    return u;
  }
}

/* Returns 1 once in every n calls, returns 0 otherwise */
int randomMangler(int n){
  if(n < 2)
    return n;
  else
    return (rand()%n)/(n-1);
}

void testCallTrace(){
  CAM_init(CAM_LOOP_PROFILE);
  for(int i = 0; i < 5; i++){
    CAM_profileLoopInvocationStart(5);
    if(i == 4)
      CAM_forceLoopTraceDump();
    CAM_profileLoopIterationStart();
    for(int j = 0; j < 5; j++){
      CAM_profileLoopSeenInstruction(9000);
      CAM_profileCallInvocationStart(9000);
      CAM_profileCallInvocationEnd();
      CAM_profileLoopSeenInstruction(9001);
      CAM_profileCallInvocationStart(9001);
      CAM_profileCallInvocationEnd();
    }
    CAM_profileLoopInvocationEnd();
  }
  for(int i = 0; i < 5; i++){
    CAM_profileLoopInvocationStart(5);
    CAM_profileLoopIterationStart();
    for(int j = 0; j < 5; j++){
      CAM_profileLoopSeenInstruction(9000);
      CAM_profileCallInvocationStart(9000);
      CAM_profileCallInvocationEnd();
      CAM_profileLoopSeenInstruction(9001);
      CAM_profileCallInvocationStart(9001);
      CAM_profileCallInvocationEnd();
    }
    CAM_profileLoopInvocationEnd();
  }
  /*
    for(int j = 0; j < 10; j++){
      CAM_profileLoopIterationStart();

      CAM_profileCallInvocationStart(9000);
      for(int k = 0; k < 10; k++){
        CAM_profileLoopSeenInstruction(9001);
        CAM_profileLoopSeenInstruction(9002);
        CAM_profileLoopSeenInstruction(9003);
        CAM_profileLoopSeenInstruction(9004);
        if(i%2==0)
          CAM_profileLoopSeenInstruction(9005);
      }
      CAM_profileCallInvocationEnd();

      CAM_profileCallInvocationStart(9010);
      for(int k = 0; k < 10; k++){
        CAM_profileLoopSeenInstruction(9001);
        CAM_profileLoopSeenInstruction(9002);
        CAM_profileLoopSeenInstruction(9003);
        CAM_profileLoopSeenInstruction(9004);
        if(i%3==0)
          CAM_profileLoopSeenInstruction(9005);
        if(j%2 == 0)
          CAM_profileLoopSeenInstruction(9009);
      }
      CAM_profileCallInvocationEnd();


      CAM_profileLoopSeenInstruction(1000);
      CAM_profileLoopSeenInstruction(2000);
      CAM_profileLoopSeenInstruction(3000);
      CAM_profileLoopSeenInstruction(4000);
      CAM_profileLoopSeenInstruction(5000);
      if(j%2 == 0)
        CAM_profileLoopSeenInstruction(5000);
    }
    if(i%2 == 0)
      CAM_profileLoopSeenInstruction(6000);
  }
  */
  CAM_shutdown(CAM_LOOP_PROFILE);
}

void testAllocator(){
  CamMemoryAllocator alloc;
  XanBitSet* bset = alloc.allocBitSet(256);
  alloc.setBit(bset, 0);
  alloc.setBit(bset, 1024);
  alloc.setBit(bset, 10000000);
  alloc.freeBitSet(bset);
  //XanHashTable* htable = alloc.allocHashTable(hashUint64AsPtr, matchUint64AsPtr);
  //alloc.hashTableInsert(htable, createUint64AsPtr(0), 0);
  //alloc.freeHashTable(htable);
  cout << "mem used: " << alloc.memUsed << endl;
  cout << "max mem used: " << alloc.maxMemUsed << endl;
}

void testControlFlowCompressor(){
#if 0
  ControlFlowCompressor cfc(20);

  for(int i = 0; i < 5; i++){
    cfc.insertSymbol(2);
  }
  cout << cfc << endl;

  ifstream instrTrace("instruction_trace.txt");
  c_symbol sym;
  unsigned int count = 0;
  while(instrTrace >> sym){
    cfc.insertSymbol(sym);
    count ++;
    if(count %10000 == 0) cout << count << endl;
  }
  ofstream compressedOutput("instruction_trace_compressed.txt");
  compressedOutput << cfc << endl;
  compressedOutput.close();

  ofstream decompressedOutput("instruction_trace_decompressed.txt");
  vector<c_symbol> decomp = cfc.decompress();
  for(auto sym = decomp.begin(); sym != decomp.end(); sym++)
    decompressedOutput << *sym << endl;
  decompressedOutput.close();
  

#endif
  vector<tracer_symbol> inputSyms;
  srand (time(NULL));
  CamMemoryAllocator alloc;
  ControlFlowCompressor *cfc;
  cfc = alloc.newMem<ControlFlowCompressor>(&alloc, 20);

  for(int asdf = 0; asdf < 1; asdf++){
  for(int j = 0; j < 1; j++){
    for(int i = 0; i < 1; i++){

      inputSyms.push_back(50);

      inputSyms.push_back(10);
      inputSyms.push_back(20);
      inputSyms.push_back(30);
      inputSyms.push_back(40);

      inputSyms.push_back(10);
      inputSyms.push_back(20);
      inputSyms.push_back(30);
      inputSyms.push_back(40);

      inputSyms.push_back(10);
      inputSyms.push_back(20);
      inputSyms.push_back(30);
      inputSyms.push_back(40);

      inputSyms.push_back(10);
      inputSyms.push_back(20);
      inputSyms.push_back(30);
      inputSyms.push_back(40);

      inputSyms.push_back(50);
      inputSyms.push_back(50);

    }
    inputSyms.push_back(99);
  }
  //for(int i = 0; i < 10; i++){
  //  inputSyms.push_back(rand()%10);
  //}
  }
  for(auto i = inputSyms.begin(); i != inputSyms.end(); i++)
    cfc->insertSymbol(*i);
  cout << (cfc->decompress() == inputSyms ? "Passed" : "Failed") << endl;
  cout << *cfc << endl;
  alloc.deleteMem(cfc);
#if 0
   
  vector<CompressionPattern> v;
  for(int i = 1; i < 10; i++){
    v.push_back(CompressionPattern(1000*i));
  }
  CompressionPattern p(v, 5);
  vector<CompressionPattern> v1;
  v1.push_back(CompressionPattern(105));
  v1.push_back(CompressionPattern(106));
  v1.push_back(p);
  CompressionPattern p2(v1, 10);
  cout << p2 << endl;
  CompressionPattern p1(v, 10);
  p1 = p;
  cout << p1 << endl << p << endl;
  p1 = p2;
  cout << p1 << endl;
#endif
}

#if 0
void testAddressTrace(){
  AddressTrace trace;
  for(int i = 0; i < 10; i++){
    trace.insertAddress(10000*(i+1), 4, 5);
    trace.insertAddress(10000*(i+1), 4, 6);
    trace.insertAddress(10000*(i+1), 4, 7);
    trace.insertAddress(10000*(i+1), 4, 8);
    trace.insertAddress(10000*(i+1), 4, 9);
    trace.insertAddress(10000*(i+1), 4, 10);
  }
  cout << trace << endl;
}
#endif

void testLoopTrace(){
  CAM_init(CAM_LOOP_PROFILE);
  for(int i = 0; i < 10; i++){
    CAM_profileLoopInvocationStart(5);
    for(int j = 0; j < 10; j++){
      CAM_profileLoopIterationStart();

      CAM_profileCallInvocationStart(9000);
      for(int k = 0; k < 10; k++){
        CAM_profileLoopSeenInstruction(9001);
        CAM_profileLoopSeenInstruction(9002);
        CAM_profileLoopSeenInstruction(9003);
        CAM_profileLoopSeenInstruction(9004);
        if(i%2==0)
          CAM_profileLoopSeenInstruction(9005);
      }
      CAM_profileCallInvocationEnd();

      CAM_profileCallInvocationStart(9010);
      for(int k = 0; k < 10; k++){
        CAM_profileLoopSeenInstruction(9001);
        CAM_profileLoopSeenInstruction(9002);
        CAM_profileLoopSeenInstruction(9003);
        CAM_profileLoopSeenInstruction(9004);
        if(i%3==0)
          CAM_profileLoopSeenInstruction(9005);
        if(j%2 == 0)
          CAM_profileLoopSeenInstruction(9009);
      }
      CAM_profileCallInvocationEnd();


      CAM_profileLoopSeenInstruction(1000);
      CAM_profileLoopSeenInstruction(2000);
      CAM_profileLoopSeenInstruction(3000);
      CAM_profileLoopSeenInstruction(4000);
      CAM_profileLoopSeenInstruction(5000);
      if(j%2 == 0)
        CAM_profileLoopSeenInstruction(5000);
    }
    if(i%2 == 0)
      CAM_profileLoopSeenInstruction(6000);
    CAM_profileLoopInvocationEnd();
  }
  CAM_shutdown(CAM_LOOP_PROFILE);
}

void testLoopTraceLarge(){
  srand(time(NULL));
  CAM_init(CAM_LOOP_PROFILE);
  for(int i = 0; i < 100; i++){
    //cout << "\r" << i; cout.flush();
    CAM_profileLoopInvocationStart(5);
    for(int j = 0; j < 10; j++){
      CAM_profileLoopIterationStart();
      for(int k = 0; k < 10; k++){
        //CAM_profileLoopSeenInstruction((rand()%1000)+5000);
        CAM_profileLoopSeenInstruction(5000+k);
      }
    }
    CAM_profileLoopSeenInstruction(5000+(i));
    CAM_profileLoopInvocationEnd();
  }
  CAM_shutdown(CAM_LOOP_PROFILE);
}

void testMemoryTrace(){
  CAM_init(CAM_MEMORY_PROFILE);
  CAM_mem(1000, 20, 1, 30, 1, 0xffffff1c, 4);
  CAM_mem(1000, 21, 1, 31, 1, 0xffffff18, 4);
  CAM_mem(1000, 22, 1, 32, 1, 0xffffff14, 4);
  CAM_mem(1000, 0, 0, 0, 0, 0xffffff10, 4);
  CAM_mem(1000, 0, 0, 0, 0, 0xffffff0c, 4);
  CAM_mem(1000, 0, 0, 0, 0, 0xffffff08, 4);
  CAM_mem(1000, 0, 0, 0, 0, 0xffffff04, 4);
  CAM_mem(1000, 0, 0, 0, 0, 0xffffff00, 4);
  for(int i = 0; i < 20; i++)
    CAM_mem(2000, 50000 + rand()%1000, 4, 0, 0, 0, 0);
  CAM_shutdown(CAM_MEMORY_PROFILE);
  cout << "testMemoryTrace complete\n";
  MemoryTrace m = parse_memory_trace();
  cout << m << endl;
}

void startInv(vector<vector<vector<uintptr_t>>>& input, vector<vector<map<uintptr_t, uint64_t>>>& inputCounts){
  CAM_profileLoopInvocationStart(133);
  input.push_back(vector<vector<uintptr_t>>());
  inputCounts.push_back(vector<map<uintptr_t, uint64_t>>());
}
void endInv(vector<vector<vector<uintptr_t>>>& input, vector<vector<map<uintptr_t, uint64_t>>>& inputCounts){
  CAM_profileLoopInvocationEnd();
}
void startIter(vector<vector<vector<uintptr_t>>>& input, vector<vector<map<uintptr_t, uint64_t>>>& inputCounts){
  CAM_profileLoopIterationStart();
  input.back().push_back(vector<uintptr_t>());
  inputCounts.back().push_back(map<uintptr_t, uint64_t>());
}
void seenInst(uintptr_t ID, vector<vector<vector<uintptr_t>>>& input, vector<vector<map<uintptr_t, uint64_t>>>& inputCounts){
  CAM_profileLoopSeenInstruction(ID);
  input.back().back().push_back(ID);
  inputCounts.back().back()[ID]++;
}

void loopTraceRandomTest_impl(int num_instructions, int num_instances, int num_invocations, int num_iterations, int num_dumps, int tag=0){
  cout << " num_instructions: " << num_instructions 
       << " num_instances: " << num_instances
       << " num_invocations: " << num_invocations 
       << " num_iterations: " << num_iterations  
       << " num_dumps: " << num_dumps  << endl;

  set<uintptr_t> ids;

  cout << "simulating trace\n";
  vector<vector<vector<uintptr_t>>> input;
  vector<vector<map<uintptr_t, uint64_t>>> inputCounts;
  CAM_init(CAM_LOOP_PROFILE);
  startInv(input, inputCounts);
  startIter(input, inputCounts);
  for(int i = 0; i < num_instances; i++){
    if(randomMangler(num_instances/num_invocations)){
      endInv(input, inputCounts);
      startInv(input, inputCounts);
      startIter(input, inputCounts);
      continue;
    }
    if(randomMangler(num_instances/(num_invocations*num_iterations))){
      startIter(input, inputCounts);
      continue;
    }
    if(randomMangler(num_instances/num_dumps)){
      CAM_forceLoopTraceDump();
      continue;
    }
    uintptr_t ID = (1 + i%num_instructions) + randomMangler(100);
    ids.insert(ID);
    seenInst(ID, input, inputCounts);
  }
  endInv(input, inputCounts);
  CAM_shutdown(CAM_LOOP_PROFILE);

  cout << "parsing\n";
  vector<vector<vector<uintptr_t>>> output;
  vector<vector<map<uintptr_t, uint64_t>>> outputCounts;
  pair<set<uintptr_t>, uint64_t> firstPass = parseLoopTraceForInstrList();
  StreamParseLoopRec sloop;
  uint64_t invocationsParsed = 0;
  for(auto invi = sloop.ii_begin(); invi != sloop.ii_end(); invi = sloop.ii_next(invi)){
    invocationsParsed++;
    output.push_back(vector<vector<uintptr_t>>());
    outputCounts.push_back(vector<map<uintptr_t, uint64_t>>());
    InvocationGroupCfc& invGroup = *invi.first->getInvocationGroupPointer();
    for(auto iteri = invGroup.iterationIteratorBegin(); iteri != invGroup.iterationIteratorEnd(); iteri = invGroup.iterationIteratorNext(iteri)){
      outputCounts.back().push_back(map<uintptr_t, uint64_t>());
      for(auto id : ids){
        if(invGroup.getNumInstancesFromII(iteri, id) > 0)
          outputCounts.back().back()[id] += invGroup.getNumInstancesFromII(iteri, id);
      }
    }
  }

  if(tag)
    if(system(("mv loop_trace.txt.bz2 loop_trace." + to_string(tag) + ".txt.bz2").c_str())) abort();

  cout << "verifying\n";
  if(inputCounts.size() != outputCounts.size()) { cout << "Number of invocations mismatch\n"; abort(); }
  if(invocationsParsed != firstPass.second) { cout << "First pass number of invocations mismatch\n"; abort(); }
  for(auto i = inputCounts.begin(), o = outputCounts.begin(); i != inputCounts.end(); i++, o++){
    if(i->size() != o->size()) { cout << "Number of iterations mismatch: " << i->size() << ", " << o->size() << endl; abort(); }
    for(auto i2 = i->begin(), o2 = o->begin(); i2 != i->end(); i2++, o2++){
      if(i2->size() != o2->size()) { cout << "Number of instruction ids mismatch: " << i2->size() << ", " << o2->size() << endl; abort(); }
      for(auto i3 = i2->begin(), o3 = o2->begin(); i3 != i2->end(); i3++, o3++){
        if(*i3 != *o3) { cout << "Nope: " << i3->first << " (" << i3->second << ") " << o3->first << " (" << o3->second << ")\n"; abort();}
      }
    }
  }

  cout << "SUCCESS!\n";
}

void loopTraceRandomTest(){
  cout << " ** Loop trace random test **\n";

  loopTraceRandomTest_impl(5, 10000, 10, 10, 2, 1);
  loopTraceRandomTest_impl(5, 100000, 100, 100, 2, 2);
  loopTraceRandomTest_impl(5, 100000, 10000, 1, 2, 3);
  loopTraceRandomTest_impl(5, 100000, 1, 10000, 2, 4);
  loopTraceRandomTest_impl(5, 10000000, 1, 10000, 10, 5);
  loopTraceRandomTest_impl(5, 10000, 10, 100, 1000, 6);

}

void startCallLoopInv(vector<vector<vector<pair<uintptr_t, vector<uintptr_t>>>>>& input, 
                      vector<vector<map<uintptr_t, map<uintptr_t, uint64_t>>>>& inputCounts){
  CAM_profileLoopInvocationStart(133);
  input.push_back(vector<vector<pair<uintptr_t, vector<uintptr_t>>>>());
  inputCounts.push_back(vector<map<uintptr_t, map<uintptr_t, uint64_t>>>());
}

void endCallLoopInv(){
  CAM_profileLoopInvocationEnd();
}

void startCallLoopIter(vector<vector<vector<pair<uintptr_t, vector<uintptr_t>>>>>& input, 
                       vector<vector<map<uintptr_t, map<uintptr_t, uint64_t>>>>& inputCounts){
  CAM_profileLoopIterationStart();
  input.back().push_back(vector<pair<uintptr_t, vector<uintptr_t>>>());
  inputCounts.back().push_back(map<uintptr_t, map<uintptr_t, uint64_t>>());
}


void startCallInv(vector<vector<vector<pair<uintptr_t, vector<uintptr_t>>>>>& input, 
                  vector<vector<map<uintptr_t, map<uintptr_t, uint64_t>>>>& inputCounts, uintptr_t callid){
  CAM_profileLoopSeenInstruction(callid);
  CAM_profileCallInvocationStart(callid);
  input.back().back().push_back(pair<uintptr_t, vector<uintptr_t>>(callid, vector<uintptr_t>()));
  if(inputCounts.back().back().find(callid) == inputCounts.back().back().end())
    inputCounts.back().back()[callid] = map<uintptr_t, uint64_t>();
}

void endCallInv(){
  CAM_profileCallInvocationEnd();
  //input.back().push_back(vector<uintptr_t>());
  //inputCounts.back().push_back(map<uintptr_t, uint64_t>());
}

void callSeenInst(vector<vector<vector<pair<uintptr_t, vector<uintptr_t>>>>>& input, 
                  vector<vector<map<uintptr_t, map<uintptr_t, uint64_t>>>>& inputCounts, uintptr_t id){
  CAM_profileLoopSeenInstruction(id);
  input.back().back().back().second.push_back(id);
  inputCounts.back().back()[input.back().back().back().first][id]++;
}

void callTraceRandomTest_impl(
      int num_calls
    , int num_instructions
    , int num_instances
    , int num_loopinvocations
    , int num_loopiterations
    , int num_callinvocations
    , int num_dumps
    , int tag=0){
  cout << " num_calls: " << num_calls 
       << " num_instructions: " << num_instructions 
       << " num_instances: " << num_instances
       << " num_loopinvocations: " << num_loopinvocations 
       << " num_loopiterations: " << num_loopiterations 
       << " num_callinvocations: " << num_callinvocations  
       << " num_dumps: " << num_dumps  << endl;

  set<uintptr_t> ids;
  set<uintptr_t> callids;
  callids.insert(901);

  bool printTrace = false;

  cout << "simulating trace\n";
  vector<vector<vector<pair<uintptr_t, vector<uintptr_t>>>>> input;
  vector<vector<map<uintptr_t, map<uintptr_t, uint64_t>>>> inputCounts;
  CAM_init(CAM_LOOP_PROFILE);
  startCallLoopInv(input, inputCounts);
  startCallLoopIter(input, inputCounts);
  startCallInv(input, inputCounts, 901);
  uint64_t invocationNum = 0;
  uint64_t iterationNum = 0;
  for(int i = 0; i < num_instances; i++){
    if(randomMangler(num_instances/num_loopinvocations)){
      invocationNum++;
      iterationNum = 0;
      endCallInv();
      endCallLoopInv();
      startCallLoopInv(input, inputCounts);
      startCallLoopIter(input, inputCounts);
      if(printTrace){
        cout << "invocation " << invocationNum << endl;;
        cout << "  iteration 0\n";
        cout << "    call (" << 901 << ")\n";
      }
      startCallInv(input, inputCounts, 901);
      continue;
    }
    if(randomMangler(num_instances/(num_loopinvocations*num_loopiterations))){
      iterationNum++;
      endCallInv();
      startCallLoopIter(input, inputCounts);
      uintptr_t callID = (901 + i%num_calls) + randomMangler(100);
      callids.insert(callID);
      if(printTrace){
        cout << "  iteration " << iterationNum << endl;
        cout << "    call (" << callID << ")\n";
      }
      startCallInv(input, inputCounts, callID);
      continue;
    }
    if(randomMangler(num_instances/(num_loopinvocations*num_loopiterations*num_callinvocations))){
      endCallInv();
      uintptr_t callID = (901 + i%num_calls) + randomMangler(100);
      callids.insert(callID);
      startCallInv(input, inputCounts, callID);
      if(printTrace){
        cout << "    call (" << callID << ")\n";
      }
      continue;
    }
    if(randomMangler(num_instances/num_dumps)){
      CAM_forceLoopTraceDump();
      continue;
    }
    uintptr_t ID = (1 + i%num_instructions) + randomMangler(100);
    ids.insert(ID);
    callSeenInst(input, inputCounts, ID);
    if(printTrace){
      cout << "      instruction (" << ID << ")\n";
    }
  }
  endCallInv();
  endCallLoopInv();
  CAM_shutdown(CAM_LOOP_PROFILE);

  cout << "parsing\n";
  vector<vector<map<uintptr_t, map<uintptr_t, uint64_t>>>> outputCounts;
  map<uintptr_t, uint64_t> subInstrRunningCount;
  pair<set<uintptr_t>, set<uintptr_t>> instrIDs = parseCallTraceForInstrList();
  pair<set<uintptr_t>, uint64_t> firstPass = parseLoopTraceForInstrList();
  StreamParseCallTrace ct = parseCallTrace();
  ct.buildLuts();

  StreamParseLoopRec sloop;
  invocationNum = 0;
  for(auto invi = sloop.ii_begin(); invi != sloop.ii_end(); invi = sloop.ii_next(invi)){
    CallTraceLoopInvocationGroup invocCallTrace = ct.getInvocCallTraceFromInvocNumber(invi.second);
    InvocationGroupCfc* invocGroup = invi.first->getInvocationGroupPointer();
    invocCallTrace.buildCallTraceCache(*invocGroup, subInstrRunningCount, callids);
    outputCounts.push_back(vector<map<uintptr_t, map<uintptr_t, uint64_t>>>());
    outputCounts.back().resize(invocGroup->getNumberOfIterations());
    for(auto instrID : ids){
      for(  auto cii = invocCallTrace.callInstanceIteratorBegin(instrID); 
            cii != invocCallTrace.callInstanceIteratorEnd(instrID); 
            cii = invocCallTrace.callInstanceIteratorNext(cii)){
        uintptr_t callid = invocCallTrace.getCallIDFromCII(cii);
        uint64_t iter = invocGroup->getIterationNumber(callid, invocCallTrace.getCallInstanceFromCII(cii));
        uint64_t numinst = invocCallTrace.getNumInstancesFromCII(cii);
        //if(invocationNum == 328){
        //  invocGroup->printInstructionInstancesLut();
        //  exit(0);
        //}
        outputCounts.back()[iter][callid][instrID] += numinst;
      }
    }
    invocationNum++;
  }

  if(tag){
    if(system(("mv call_trace.txt.bz2 call_trace." + to_string(tag) + ".txt.bz2").c_str())) abort();
    if(system(("mv loop_trace.txt.bz2 loop_trace." + to_string(tag) + ".txt.bz2").c_str())) abort();
  }

  cout << "verifying\n";
  uint64_t invocNum = 0;
  if(inputCounts.size() != outputCounts.size()) { cout << "Number of invocations mismatch\n"; abort(); }
  for(auto i = inputCounts.begin(), o = outputCounts.begin(); i != inputCounts.end(); i++, o++){
    if(i->size() != o->size()) { 
      cout << "Number of iterations mismatch: " << i->size() << ", " << o->size() << endl; 
      cout << "Invocation " << invocNum << endl;
      abort(); 
    }
    uint64_t iterNum = 0;
    for(auto i2 = i->begin(), o2 = o->begin(); i2 != i->end(); i2++, o2++){
      //if(i2->size() != o2->size()) { 
      //  cout << "Number of call ids mismatch: " << i2->size() << ", " << o2->size() << " (" << invocNum << "," << iterNum << ")" << endl; 
      //  for(auto x : *i2) cout << x.first << "(" << x.second.size() << ") ";
      //  cout << endl;
      //  for(auto x : *o2) cout << x.first << "(" << x.second.size() << ") ";
      //  cout << endl;
      //  abort(); 
      //}
      for(auto i3 = i2->begin(); i3 != i2->end(); i3++){
        auto o3 = o2->find(i3->first);
        if(o3 == o2->end()) continue;
        if(i3->first != o3->first) { cout << "Call id mismatch: " << i3->first << ", " << o3->first << endl; abort(); }
        if(i3->second.size() != o3->second.size()) { 
          cout << "Number of sub ids mismatch for call: " << i3->first << "(" << i3->second.size() << ", " << o3->second.size() << ")" << endl; 
          cout << "  invocation " << invocNum << endl
               << "  iteration " << iterNum << endl;
          for(auto x : o3->second)
            cout << x.first << ":" << x.second << endl;
          abort();
        }
        for(auto i4 = i3->second.begin(), o4 = o3->second.begin(); i4 != i3->second.end(); i4++, o4++){
          if(i4->first != o4->first) { cout << "Sub id mismatch: " << i4->first << ", " << o4->first << endl; abort(); }
          if(i4->second != o4->second) { 
            cout << "Number of instances mismatch for sub: " << i4->first << "(" << i4->second << ", " << o4->second << ")" << endl; 
            cout << "  invocation " << invocNum << endl
                 << "  iteration " << iterNum << endl
                 << "  callid " << i3->first << endl;
            abort();
          }
        }
      }
      iterNum++;
    }
    invocNum++;
  }

  cout << "SUCCESS!\n";
}

void callTraceRandomTest(){
  cout << " ** Call trace random test **\n";

  callTraceRandomTest_impl(1, 5, 10000, 10, 10, 10, 2, 1);
  callTraceRandomTest_impl(5, 1, 1000000, 100, 10, 100, 2, 2);
  callTraceRandomTest_impl(5, 5, 1000000, 10000, 10, 1, 2, 3);
  callTraceRandomTest_impl(5, 5, 1000000, 1, 10, 10000, 2, 4);
  callTraceRandomTest_impl(5, 5, 100000, 1, 10, 10, 1000, 4);
}

void memoryTraceRandomTest(int compressibility){
  int num_instructions = 500;
  uintptr_t min_address = 1000000;
  uintptr_t address_range = 1000000;
  int num_instances = 1000000;
  int ave_num_dumps = 10;

  cout << " ** Memory trace random test **\n";

  cout << "simulating trace\n";
  map<uintptr_t, vector<uintptr_t>> input;
  CAM_init(CAM_MEMORY_PROFILE);
  for(int i = 0; i < num_instances; i++){
    uintptr_t ID = (1 + rand()%num_instructions)*1000;
    uintptr_t value;
    if(compressibility == 0)
      value = min_address + rand()%address_range;
    else
      value = min_address + (rand()%10)/9;
    input[ID].push_back(value);
    if(ID%2000 == 0)
      CAM_mem(ID, value, 4, 0, 0, 0, 0);
    else if(ID%5000 == 0)
      CAM_mem(ID, value, 4, 0, 0, value, 4);
    else
      CAM_mem(ID, 0, 0, 0, 0, value, 4);
    if((rand()%num_instances) < ave_num_dumps)
      CAM_forceMemTraceDump();
  }
  CAM_shutdown(CAM_MEMORY_PROFILE);

  cout << "parsing\n";
  MemoryTrace m = parse_memory_trace();
  //MemoryTrace m = parse_memory_trace_parallel();

  cout << "verifying\n";
  map<uintptr_t, vector<uintptr_t>> output;
  for(auto inst = m.begin(); inst != m.end(); inst++){
    uintptr_t ID = inst->first;
    /* Read instructions and write instructions */
    if(ID%2000 == 0 || ID%5000 != 0){
      MemSet& set = ID%2000 == 0 ? inst->second.readSet : inst->second.writeSet;
      for(auto e = set.begin(); e != set.end(); e++){
        for(uint64_t numRep = 0; numRep < e->getNumInstances(); numRep++)
          output[ID].push_back(e->getAccessLower(numRep));
      }
    }
    /* Read/Write instructions */
    else{
      MemSet& rset = inst->second.readSet;
      MemSet& wset = inst->second.writeSet;
      if(rset.size() != wset.size()){
        cout << "Read/write instruction " << ID << " has read and write sets of different lengths\n";
        cout << rset << endl;
        cout << wset << endl;
        abort();
      }
      for(auto re = rset.begin(), we = wset.begin(); re != rset.end(); re++, we++){
        for(uint64_t numRep = 0; numRep < re->getNumInstances(); numRep++){
          if(re->getAccessLower(numRep) != we->getAccessLower(numRep)){
            cout << "Mismatch in r/w " << ID << ": " << re->getAccessLower(numRep) << "," << we->getAccessLower(numRep) << endl;
            abort();
          }
          output[ID].push_back(re->getAccessLower(numRep));
        }
      }
    }
  }
  assert(input.size() == output.size());
  for(auto in = input.begin(), out = output.begin(); in != input.end(); in++, out++){
    if(in->first != out->first){
      cout << "instruction ID mismatch: " << in->first << ", " << out->first << endl;
      abort();
    }
    //cout << " ** " << in->first << endl;
    for(auto i = in->second.begin(), o = out->second.begin(); i != in->second.end(); i++, o++){
      //cout << i - in->second.begin() << ": " << *i << ":" << *o << endl;
      if(*i != *o){
        cout << "instrID: " << in->first << endl;
        cout << *i << ", " << *o << endl;
        abort();
      }
    }
  }
  cout << "SUCCESS!\n";
}

void testCallTraceLarge(){
  srand(time(NULL));
  CAM_init(CAM_LOOP_PROFILE);
  for(int i = 0; i < 100; i++){
    CAM_profileLoopInvocationStart(5);
    for(int j = 0; j < 100; j++){
      CAM_profileCallInvocationStart(1111);
      CAM_profileLoopSeenInstruction(11110);
      //for(int k = 0; k < 20; k++){
      //  CAM_profileLoopSeenInstruction((rand()%1000)+5000);
      //}
      if(rand()%1000 == 0)
        CAM_forceLoopTraceDump();
      CAM_profileCallInvocationEnd();
      CAM_profileCallInvocationStart(1112);
      CAM_profileLoopSeenInstruction(11120);
      if(rand()%1000 == 0)
        CAM_forceLoopTraceDump();
      CAM_profileCallInvocationEnd();
      CAM_profileCallInvocationStart(1113);
      CAM_profileLoopSeenInstruction(11130);
      if(rand()%1000 == 0)
        CAM_forceLoopTraceDump();
      CAM_profileCallInvocationEnd();
    }
    CAM_profileLoopInvocationEnd();
  }
  CAM_shutdown(CAM_LOOP_PROFILE);
}

void testHashTable(){
  XanHashTable* table = xanHashTable_new(11, JITFALSE, malloc, realloc, free, hashUint64AsPtr, matchUint64AsPtr);
  xanHashTable_insert(table, (void*)0x811b248, (void*)0x811f30c);
  xanHashTable_insert(table, (void*)0x811d3b8, (void*)0x811f134);
  xanHashTable_insert(table, (void*)0x811c3c8, (void*)0x811f164);
  cout << xanHashTable_lookupItem(table, (void*)0x811b248) << endl;
  cout << xanHashTable_lookupItem(table, (void*)0x811d3b8) << endl;
  cout << xanHashTable_lookupItem(table, (void*)0x811c3c8) << endl;
}

void testLoopTraceStreamer(uint64_t chunkSize){
  LoopTraceStreamer streamer;
  while(1){
    LoopTraceEntry lte;
    if(!streamer.getNextEntry(lte)){
      break;
    }
  }
}

void testCallTraceStreamer(uint64_t chunkSize){
  CallTraceStreamer streamer;
  while(1){
    CallTraceLoopInvocationGroup clig;
    if(!streamer.getNextEntry(clig)){
      break;
    }
    cout << clig << endl;
  }
}

void testMemoryTraceStreamer(uint64_t chunkSize){
  pair<vector<uintptr_t>, vector<uintptr_t>> instrs = findMemoryTraces();
  list<MemoryTraceStreamer*> streamers;
  for(auto id = instrs.first.begin(); id != instrs.first.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%"PRIuPTR".r.txt.bz2", *id);
    streamers.push_back( new MemoryTraceStreamer(*id, path, false));
  }
  for(auto id = instrs.second.begin(); id != instrs.second.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%"PRIuPTR".w.txt.bz2", *id);
    streamers.push_back( new MemoryTraceStreamer(*id, path, true));
  }
  while(!streamers.empty()){
    for(auto s = streamers.begin(); s != streamers.end(); ){
      if((*s)->getNextChunk(chunkSize).empty()){
        delete *s;
        s = streamers.erase(s);
      }
      else
        s++;
    }
  }
}

void getTraceStats(){
  cout << "MEMORY TRACE STATISTICS\n";
  pair<vector<uintptr_t>, vector<uintptr_t>> instrs = findMemoryTraces();
  map<pair<uintptr_t, bool>, uint64_t> memoryTraceCounts;
  for(auto id = instrs.first.begin(); id != instrs.first.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%"PRIuPTR".r.txt.bz2", *id);
    MemoryTraceStreamer s(*id, path, false);
    uint64_t count = 0;
    do{
      MemSet ms = s.getNextChunk(1000, true);
      if (ms.empty()) break;
      else count = ms.back().getEnd() + 1;
    }while(1);
    memoryTraceCounts[pair<uintptr_t, bool>(*id, false)] = count;
  }
  for(auto id = instrs.second.begin(); id != instrs.second.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%"PRIuPTR".w.txt.bz2", *id);
    MemoryTraceStreamer s(*id, path, true);
    uint64_t count = 0;
    do{
      MemSet ms = s.getNextChunk(1000, true);
      if (ms.empty()) break;
      else count = ms.back().getEnd() + 1;
    }while(1);
    memoryTraceCounts[pair<uintptr_t, bool>(*id, true)] = count;
  }
  for(auto m : memoryTraceCounts){
    cout << m.first.first << " " << (m.first.second ? "write: " : "read:  ") << m.second << endl;
  }
  cout << endl;

  cout << "LOOP TRACE STATISTICS\n";
  pair<set<uintptr_t>, uint64_t> instrListAndNumInvoc; 
  instrListAndNumInvoc = parseLoopTraceForInstrList();
  map<uintptr_t, uint64_t> loopTraceCounts;
  uint64_t numInvocationsDynamicParse = 0;
  while(1){
    LoopTraceEntry l;
    cerr << "Not currently working\n";
    abort();
    l.precomputeDataStructures();
    InvocationGroupCfc& inv = *l.getInvocationGroupPointer();
    uint64_t numInv = l.getNumberOfInvocations_slow();
    numInvocationsDynamicParse += numInv;
    for(auto i : instrListAndNumInvoc.first)
      loopTraceCounts[i] += inv.getNumInstances(i)*numInv;
  }
  for(auto l : loopTraceCounts)
    cout << l.first << ": " << l.second << endl;
  if(numInvocationsDynamicParse != instrListAndNumInvoc.second)
    cout << " *** ERROR *** Number of invocations did not match between parses: " 
         << numInvocationsDynamicParse << " - " << instrListAndNumInvoc.second << endl;

}

map<string, unsigned int> argsParser(int argc, char **argv){
  int c;
  map<string, unsigned int> args;
  args["memparse"] = 0;
  args["loopparse"] = 0;
  args["callparse"] = 0;
  args["stats"] = 0;
  args["random"] = 0;
  args["deterministic"] = 0;
  while ((c = getopt (argc, argv, "m:l:c:sr:d:")) != -1){
    switch (c) {
      case 'm':
        args["memparse"] = atoi(optarg);
        break;
      case 'l':
        args["loopparse"] = atoi(optarg);
        break;
      case 'c':
        args["callparse"] = atoi(optarg);
        break;
      case 's':
        args["stats"] = 1;
        break;
      case 'r':
        args["random"] = atoi(optarg);
        break;
      case 'd':
        args["deterministic"] = atoi(optarg);
        break;
      default:
        cerr << "Bad argument list\n";
        abort ();
    }
  }
  return args;
}

void my_handler(int s){
  printf("Caught signal %d\n",s);
  exit(1); 
}

void registerSigIntHandler(){
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
}

int main(int argc, char **argv){
  
  registerSigIntHandler();

  map<string, unsigned int> args = argsParser(argc, argv);

  if(args["deterministic"])
    srand(args["deterministic"]);
  else{
    int seed = time(NULL);
    cout << "Running with time seed: " << seed << endl;
    srand(seed);
  }

  if(args["memparse"])
    testMemoryTraceStreamer(args["memparse"]);
  else if(args["loopparse"])
    testLoopTraceStreamer(args["loopparse"]);
  else if(args["callparse"])
    testCallTraceStreamer(args["callparse"]);
  else if(args["stats"])
    getTraceStats();
  else if(args["random"]){
    if(args["random"] == 1)
      memoryTraceRandomTest(2);
    if(args["random"] == 2)
      loopTraceRandomTest();
    if(args["random"] == 3)
      callTraceRandomTest();
  }
  else
    testCallTrace();
}
