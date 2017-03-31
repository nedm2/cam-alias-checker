
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

#include "parser_wrappers.h"
#include "loop_trace_cfc_parser.h"
#include "call_trace_cfc_parser.h"
#include "memory_trace_parser.h"
#include "dependence_pairs_parser.h"
#include "static_ddg_parser.h"
#include "BZ2ParserState.h"
#include "MemoryTraceStreamer.h"
#include "LoopTraceStreamer.h"
#include "CallTraceStreamer.h"

#include <deque>
#include <vector>
#include <cassert>
#include <bzlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
using namespace std;

#if 0

enum ParseStatus {INIT, FILE_NOT_FOUND, NO_MORE_FILES, COMPLETE, INCOMPLETE, PARSECOMPLETE};

FILE *ctsp_file = NULL;

FILE *current_ct_file = NULL;
uint64_t current_ct_num = 0;
deque<CallTraceLoopInvocationGroup> pendingCallTraceInvocations;
ParseStatus CTStatus = INIT;
CallTraceCfcLex current_ctcl;

FILE *current_lt_file = NULL;
uint64_t current_lt_num = 0;
deque<LoopTraceEntry> pendingLoopTraceEntries;
ParseStatus LTStatus = INIT;
LoopTraceCfcLex current_ltcl;

#endif

pair<set<uintptr_t>, uint64_t> parseLoopTraceForInstrList(){
  LoopTraceStreamer streamer;
  return streamer.parseForInstructionList();
  #if 0
  FILE *f = fopen("loop_trace.txt", "r");
  if(!f){
    cout << "Could not find loop_trace.txt\n";
    exit(-1);
  }

  LoopTraceEntry lte;
  set<uintptr_t> instrList;
  uint64_t numInvoc = 0;

  LoopTraceCfcLex ltcl;
  ltcl.loopEntry = &lte;
  ltcl.instructionList = &instrList;
  ltcl.numInvoc = &numInvoc;
  ltcl.firstPass = true;

  looptracecfcin = f;
  int remaining = 1;
  while(remaining)
    remaining = looptracecfclex(&ltcl);
  fclose(f);
  return pair<set<uintptr_t>, uint64_t>(instrList, numInvoc+1);
  #endif
}

#if 0

/* 
 * Fills a call trace entry until the end of an invocation or until
 * there is nothing left in this file, 
 * may be missing elements from subsequent files.
*/
ParseStatus getCallTraceSegment(CallTraceLoopInvocationGroup& callEntry, bool topup){
  char buf[200]; //should be long enough
  if(!current_ct_file){
    //sprintf(buf, "call_trace.%llu.txt", current_ct_num);
    sprintf(buf, "call_trace.txt");
    current_ct_file = fopen(buf, "r");
    if(!current_ct_file){
      if(current_ct_num > 0)
        return NO_MORE_FILES;
      else{
        cerr << "Could not find " << buf << endl;
        abort();
        return FILE_NOT_FOUND;
      }
    }
  }

  /* Attempt to get a call trace entry */
  calltracecfcin = current_ct_file;
  current_ctcl.callEntry = &callEntry;
  current_ctcl.firstPass = false;
  int ret = calltracecfclex(&current_ctcl);

  if(ret == 1){ 
    ///* File has finished, close the file and increment the file counter */
    //fclose(current_ct_file);
    //current_ct_file = NULL;
    //current_ct_num++;
    return INCOMPLETE;
  }
  else if(ret == 2){
    return COMPLETE;
  }
  else{
    fclose(current_ct_file);
    return PARSECOMPLETE;
  }
}

#endif

#if 0

ParseStatus getLoopTraceSegment(LoopTraceEntry& loopEntry){
  char buf[200]; //should be long enough
  if(!current_lt_file){
    //sprintf(buf, "loop_trace.%llu.txt", current_lt_num);
    sprintf(buf, "loop_trace.txt");
    current_lt_file = fopen(buf, "r");
    if(!current_lt_file){
      if(current_lt_num > 0)
        return NO_MORE_FILES;
      else{
        cerr << "Could not find " << buf << endl;
        abort();
        return FILE_NOT_FOUND;
      }
    }
  }

  /* Attempt to get a loop trace entry */
  looptracecfcin = current_lt_file;
  current_ltcl.loopEntry = &loopEntry;
  current_ltcl.instructionList = NULL;
  current_ltcl.numInvoc = NULL;
  current_ltcl.firstPass = false;
  int ret = looptracecfclex(&current_ltcl);

  if(ret == 1){ 
    ///* File has finished, close the file and increment the file counter */
    //fclose(current_lt_file);
    //current_lt_file = NULL;
    //current_lt_num++;
    return INCOMPLETE;
  }
  else if(ret == 2){
    return COMPLETE;
  }
  else{
    fclose(current_lt_file);
    return PARSECOMPLETE;
  }
}

#endif

#if 0

void loopTraceInit(){
  current_lt_file = NULL;
  current_lt_num = 0;
  LTStatus = INIT;
}

void callTraceInit(){
  current_ct_file = NULL;
  current_ct_num = 0;
  current_ctcl = CallTraceCfcLex();
  CTStatus = INIT;
}

#endif

#if 0

/* 
Returns the next loop invocation group in loopEntry.
Return 1 if there are more invocations to consume, 0 if not 
*/
int loopTraceGetNextEntry(LoopTraceEntry& loopEntry){
  /* If there is more than one invocation in the pending queue, just return the oldest */
  if(pendingLoopTraceEntries.size() > 1){
    loopEntry = pendingLoopTraceEntries.back(); 
    pendingLoopTraceEntries.pop_back();
    return 1;
  }

  /* Check if the parse has completed */
  if(LTStatus == PARSECOMPLETE)
    return 0;

  /* Check if the last file has been consumed */
  #if 0
  if(LTStatus == NO_MORE_FILES){
    if(!pendingLoopTraceEntries.empty()){
      loopEntry = pendingLoopTraceEntries.back();
      pendingLoopTraceEntries.pop_back();
      cerr << "I did not think it was possible to get here\n";
      abort();
    }
    return 0;
  }
  #endif

  /* Get the next segment into temporary storage */
  LoopTraceEntry tempCallEntry;
  LTStatus = getLoopTraceSegment(tempCallEntry);

  /* If the result was empty, the parse is complete, assert this and return any pending */
  if(tempCallEntry.isEmpty()){
    assert(LTStatus == PARSECOMPLETE);
    loopEntry = pendingLoopTraceEntries.back();
    pendingLoopTraceEntries.pop_back();
    return 1;
  }

  /* Check for a pending call which doesn't overlap this new segment */
  if(pendingLoopTraceEntries.size() > 0){
    if(!pendingLoopTraceEntries.back().lastAndFirstInvocationOverlap(tempCallEntry)){
      /* Pending invocation was in fact complete, copy it into loopEntry and replace pending invocation */
      loopEntry = pendingLoopTraceEntries.back();
      pendingLoopTraceEntries.pop_back();
      pendingLoopTraceEntries.push_front(tempCallEntry);
      return 1;
    }
  }

  /* The new entry must overlap pending (or pending is empty), merge and check if there is anything left over */
  vector<LoopTraceEntry> leftOverLoopEntries;
  if(!pendingLoopTraceEntries.empty())
    leftOverLoopEntries = pendingLoopTraceEntries.back().mergeIntoAndReturnRemaining(tempCallEntry);
  else
    pendingLoopTraceEntries.push_front(tempCallEntry);
  if(!leftOverLoopEntries.empty()){
    /* Stuff was left over after the merge, therefore what's in pendingCallTraceInvocation must be complete */
    loopEntry = pendingLoopTraceEntries.back();
    pendingLoopTraceEntries.pop_back();
    copy(leftOverLoopEntries.begin(), leftOverLoopEntries.end(), front_inserter(pendingLoopTraceEntries));
    return 1;
  }

  /* Continue to consume files until we can be sure we have the entire invocation */
  leftOverLoopEntries.clear();
  while(leftOverLoopEntries.empty() && LTStatus == INCOMPLETE){
    tempCallEntry.clear();
    LTStatus = getLoopTraceSegment(tempCallEntry);
    if(pendingLoopTraceEntries.empty())
      pendingLoopTraceEntries.push_front(tempCallEntry);
    else{
      leftOverLoopEntries = pendingLoopTraceEntries.back().mergeIntoAndReturnRemaining(tempCallEntry);
    }
  }

  /* Either there was a bit of the previous file left over or we've run out of files */
  loopEntry = pendingLoopTraceEntries.back();
  pendingLoopTraceEntries.pop_back();
  if(!leftOverLoopEntries.empty())
    copy(leftOverLoopEntries.begin(), leftOverLoopEntries.end(), front_inserter(pendingLoopTraceEntries));
  return 1;
}

#endif

#if 0

/* Return 1 if a valid group was returned, 0 if not */
int callTraceGetNextEntry(CallTraceLoopInvocationGroup& callEntry){
  /* If there is more than one invocation in the pending queue, just return the oldest */
  if(pendingCallTraceInvocations.size() > 1){
    callEntry = pendingCallTraceInvocations.back(); 
    pendingCallTraceInvocations.pop_back();
    return 1;
  }

  /* Check if the parse has completed */
  if(CTStatus == PARSECOMPLETE)
    return 0;

#if 0
  /* Check if the last file has been consumed */
  if(CTStatus == NO_MORE_FILES){
    if(!pendingCallTraceInvocations.empty()){
      callEntry = pendingCallTraceInvocations.back();
      pendingCallTraceInvocations.pop_back();
      cerr << "I did not think it was possible to get here\n";
      abort();
    }
    return 0;
  }
#endif

  /* Get the next segment into temporary storage */
  CallTraceLoopInvocationGroup tempCallEntry;
  CTStatus = getCallTraceSegment(tempCallEntry, false);

  /* If the result was empty, there are no more files, assert this and return any pending */
  if(tempCallEntry.isEmpty()){
    if(pendingCallTraceInvocations.size() > 0){
      callEntry = pendingCallTraceInvocations.back();
      pendingCallTraceInvocations.pop_back();
      return 1;
    }
    else if(CTStatus == PARSECOMPLETE)
      return 0;
    else
      return 1;
  }

  /* Check for a pending call which doesn't overlap this new segment */
  if(pendingCallTraceInvocations.size() > 0){
    if(!pendingCallTraceInvocations.back().lastAndFirstInvocationOverlap(tempCallEntry)){
      /* Pending invocation was in fact complete, copy it into callEntry and replace pending invocation */
      callEntry = pendingCallTraceInvocations.back();
      pendingCallTraceInvocations.pop_back();
      pendingCallTraceInvocations.push_front(tempCallEntry);
      return 1;
    }
  }

  /* The new entry must overlap pending (or pending is empty), merge and check if there is anything left over */
  vector<CallTraceLoopInvocationGroup> leftOverCallEntries;
  if(!pendingCallTraceInvocations.empty())
    leftOverCallEntries = pendingCallTraceInvocations.back().mergeIntoAndReturnRemaining(tempCallEntry);
  else
    pendingCallTraceInvocations.push_front(tempCallEntry);
  if(!leftOverCallEntries.empty()){
    /* Stuff was left over after the merge, therefore what's in pendingCallTraceInvocation must be complete */
    callEntry = pendingCallTraceInvocations.back();
    pendingCallTraceInvocations.pop_back();
    copy(leftOverCallEntries.begin(), leftOverCallEntries.end(), front_inserter(pendingCallTraceInvocations));
    return 1;
  }

  /* Continue to consume files until we can be sure we have the entire invocation */
  leftOverCallEntries.clear();
  while(leftOverCallEntries.empty() && CTStatus == INCOMPLETE){
    tempCallEntry.clear();
    CTStatus = getCallTraceSegment(tempCallEntry, true);
    if(pendingCallTraceInvocations.empty())
      pendingCallTraceInvocations.push_front(tempCallEntry);
    else{
      leftOverCallEntries = pendingCallTraceInvocations.back().mergeIntoAndReturnRemaining(tempCallEntry);
    }
  }

  /* Either there was a bit of the previous file left over or we've run out of files */
  callEntry = pendingCallTraceInvocations.back();
  pendingCallTraceInvocations.pop_back();
  if(!leftOverCallEntries.empty())
    copy(leftOverCallEntries.begin(), leftOverCallEntries.end(), front_inserter(pendingCallTraceInvocations));
  return 1;
}

#endif

StreamParseCallTrace parseCallTrace(){
  CallTraceStreamer streamer;
  StreamParseCallTrace t;
  while(1){
    CallTraceLoopInvocationGroup clig;
    int valid = streamer.getNextEntry(clig);
    if(valid)
      t.addNewInvocationGroup(clig);
    else
      break;
  }
  return t;
}

pair<set<uintptr_t>, set<uintptr_t>> parseCallTraceForInstrList(){
  CallTraceStreamer streamer;
  return streamer.parseForInstructionLists();
}

/*
 * Return two vectors: read instruction IDs, write instruction IDs
*/
pair<vector<uintptr_t>, vector<uintptr_t>> findMemoryTraces(){
  pair<vector<uintptr_t>, vector<uintptr_t>> instrIDs;
  char path[1024];
  FILE *fp  = popen("find memory_accesses -name memory_accesses.*.r.txt.bz2", "r");
  if(fp == NULL){
    perror("Failed to popen find");
    abort();
  }
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    int id;
    sscanf(path, "memory_accesses/memory_accesses.%i.r.txt.bz2", &id);
    instrIDs.first.push_back(id);
  }
  pclose(fp);
  fp  = popen("find memory_accesses -name memory_accesses.*.w.txt.bz2", "r");
  if(fp == NULL){
    perror("Failed to popen find");
    abort();
  }
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    int id;
    sscanf(path, "memory_accesses/memory_accesses.%i.w.txt.bz2", &id);
    instrIDs.second.push_back(id);
  }
  pclose(fp);
  return instrIDs;
}

MemoryTrace parse_memory_trace(){
  MemoryTrace t;
  pair<vector<uintptr_t>, vector<uintptr_t>> instrIDs = findMemoryTraces();
  for(auto id = instrIDs.first.begin(); id != instrIDs.first.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%" PRIuPTR ".r.txt.bz2", *id);
    MemoryTraceStreamer streamer(*id, path, false);
    MemSet ms;
    do{
      ms = streamer.getNextChunk(0);
      ms = streamer.getNextChunk(10);
      copy(ms.begin(), ms.end(), back_inserter(t[streamer.getID()].getReadSet()));
    } while(!ms.empty());
  }
  for(auto id = instrIDs.second.begin(); id != instrIDs.second.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%" PRIuPTR ".w.txt.bz2", *id);
    MemoryTraceStreamer streamer(*id, path, true);
    MemSet ms;
    do{
      ms = streamer.getNextChunk(0);
      ms = streamer.getNextChunk(10);
      copy(ms.begin(), ms.end(), back_inserter(t[streamer.getID()].getWriteSet()));
    } while(!ms.empty());
  }
  return t;
}

MemoryTrace parse_memory_trace_parallel(){
  MemoryTrace t;
  pair<vector<uintptr_t>, vector<uintptr_t>> instrIDs = findMemoryTraces();
  list<MemoryTraceStreamer*> streamers;
  for(auto id = instrIDs.first.begin(); id != instrIDs.first.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%" PRIuPTR ".r.txt.bz2", *id);
    streamers.push_back(new MemoryTraceStreamer(*id, path, false));
  }
  for(auto id = instrIDs.second.begin(); id != instrIDs.second.end(); id++){
    char path[1024];
    sprintf(path, "memory_accesses/memory_accesses.%" PRIuPTR ".w.txt.bz2", *id);
    streamers.push_back(new MemoryTraceStreamer(*id, path, true));
  }
  //for(auto p = parsers.begin(); p != parsers.end(); p++){
  //  (*p)->parseAll();
  //  delete *p;
  //}
  while(!streamers.empty()){
    for(auto s = streamers.begin(); s != streamers.end();){
      MemSet ms = (*s)->getNextChunk(100000000);
      if(ms.empty()){
        delete *s;
        s = streamers.erase(s);
      }
      else{
        if((*s)->isWrite())
          copy(ms.begin(), ms.end(), back_inserter(t[(*s)->getID()].getWriteSet()));
        else
          copy(ms.begin(), ms.end(), back_inserter(t[(*s)->getID()].getReadSet()));
        s++;
      }
    }
  }
  return t;
}

pair<set<pair<uintptr_t, uintptr_t>>, set<pair<uintptr_t, uintptr_t>>> parse_dependence_pairs(){
  FILE *f = fopen("dependence_pairs.txt", "r");
  if(!f){
    cout << "Could not find dependence_pairs.txt\n";
    exit(-1);
  }
  set<pair<uintptr_t, uintptr_t>> rw_pairs;
  set<pair<uintptr_t, uintptr_t>> ww_pairs;
  deppairsin = f;
  deppairslex(rw_pairs, ww_pairs);
  return pair<set<pair<uintptr_t, uintptr_t>>, set<pair<uintptr_t, uintptr_t>>>(rw_pairs, ww_pairs);
}

vector<tuple<uintptr_t, uintptr_t, unsigned int>> parse_static_ddg(){
  return parse_ddg("static_inst_dependences.txt");
}

vector<tuple<uintptr_t, uintptr_t, unsigned int>> parse_ddg(string ddgfile){
  FILE *f = fopen(ddgfile.c_str(), "r");
  if(!f){
    cout << "Could not find " << ddgfile << endl;
    exit(-1);
  }
  vector<tuple<uintptr_t, uintptr_t, unsigned int>> staticDDG;
  staticddgin = f;
  staticddglex(staticDDG);

  set<tuple<uintptr_t, uintptr_t, unsigned int>> staticDDGSet;
  copy(staticDDG.begin(), staticDDG.end(), inserter(staticDDGSet, staticDDGSet.begin()));
  if(staticDDGSet.size() != staticDDG.size())
    cerr << "WARNING: There seem to be duplicates in the static DDG\n";

  return staticDDG;
}
