
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

%top{
  #include "static_inst_rec.h"
  #include "cam_system.h"

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int memorytracelex (void *state)
	int memorytracelex (void *state);

  struct MemoryTraceLex{
    int state;
    uint64_t remaining_groups;
    int group_state;
    MemSet *memset;
    uintptr_t currBase;
    intptr_t currStride;
    uint64_t currLength;
    uint64_t currStart;
    uint64_t currEnd;
    uint64_t currNumReps;
    uintptr_t lastAccess;
    uint64_t numInstancesRequired;
    uint64_t startInstance;
    uint64_t nextExpectedInstance;
    MemoryTraceLex() : state(0), remaining_groups(0), group_state(0), lastAccess(0), startInstance(0), nextExpectedInstance(0) {}
  };
}

%{

void addNumRepsToSet(MemoryTraceLex& mtl, MemSet& set){
  set.push_back(MemSetEntry(mtl.currBase, mtl.currStride, mtl.currLength, mtl.nextExpectedInstance, mtl.nextExpectedInstance+mtl.currNumReps-1));
  mtl.nextExpectedInstance += mtl.currNumReps;
  //if(set.empty()){
  //  set.push_back(MemSetEntry(mtl.currBase, mtl.currStride, mtl.currLength, 0, mtl.currNumReps - 1));
  //}
  //else{
  //  set.push_back(MemSetEntry(mtl.currBase, mtl.currStride, mtl.currLength, set.back().getEnd()+1, set.back().getEnd()+mtl.currNumReps));
  //}
  if(set.back().isTrivialPattern()){
    set.push_back(set.back().splitTrivial());
    //vector<MemSetEntry> ms = set.back().getSplitPattern();
    //set.pop_back();
    //for(auto i = ms.begin(); i != ms.end(); i++)
    //  set.push_back(*i);
  }
}

void getAbsoluteBase(MemoryTraceLex& mtl, MemSet& set){
  intptr_t baseDiff = ALPHTOSIGNED(memorytracetext);
  if(baseDiff < 0)
    mtl.currBase = mtl.lastAccess - abs(baseDiff);
  else
    mtl.currBase = mtl.lastAccess + baseDiff;
  mtl.lastAccess = mtl.currBase;
}

void readMemSetEntry(MemoryTraceLex& mtl) {
  if(mtl.group_state == 0){
    if(mtl.state == 2) 
      getAbsoluteBase(mtl, *mtl.memset);
    else
      getAbsoluteBase(mtl, *mtl.memset);
    mtl.group_state++;
  }
  else if(mtl.group_state == 1){
    mtl.currStride = ALPHTOSIGNED(memorytracetext);
    mtl.group_state++;
  }
  else if(mtl.group_state == 2){
    mtl.currLength = strtoull(memorytracetext, NULL, 10);
    mtl.group_state++;
  }
  else if(mtl.group_state == 3){
    mtl.currNumReps = strtoull(memorytracetext, NULL, 10);
    if(mtl.state == 2)
      addNumRepsToSet(mtl, *mtl.memset);
    else
      addNumRepsToSet(mtl, *mtl.memset);
    mtl.group_state=0;
    mtl.remaining_groups--;
    if(mtl.remaining_groups == 0)
      mtl.state = (mtl.state + 1)%5;
  }
}

%}

%option noyywrap
%option noinput
%option nounput
%option header-file="memory_trace_parser.h"
%option outfile="memory_trace_parser.cpp"
%option prefix="memorytrace"

NUMBER	-?[0-9]+

%%

{NUMBER} {
  MemoryTraceLex& mtl = *(MemoryTraceLex*)(state);

  if(mtl.state == 0){
    /* The instruction ID is actually not being used any more */
    //uintptr_t instrID = ALPHTONUM(memorytracetext);
    //if(mtl.memtrace->find(instrID) == mtl.memtrace->end()){ // if this instruction has not been encountered, add a record for it
    //  (*mtl.memtrace)[instrID] = StaticInstRec();
    //}
    //mtl.currInstRec = &(*mtl.memtrace)[instrID];
    mtl.lastAccess = 0;
    mtl.state++;
  }
  else if(mtl.state == 1){
    mtl.remaining_groups = strtoull(memorytracetext, NULL, 10);
    if(mtl.remaining_groups > 0){
      mtl.group_state = 0;
      mtl.state++;
    }
    else {
      mtl.state += 2;
    }
  }
  else if(mtl.state == 2){
    readMemSetEntry(mtl);
    if(!mtl.memset->empty() && ((mtl.memset->back().getEnd() - mtl.startInstance + 1) >= mtl.numInstancesRequired)){
      return 1;
    }
  }
  else if(mtl.state == 3){
    mtl.remaining_groups = strtoull(memorytracetext, NULL, 10);
    if(mtl.remaining_groups > 0){
      mtl.group_state = 0;
      mtl.state++;
    }
    else {
      mtl.state = 0;
    }
  
  }
  else if(mtl.state == 4){
    readMemSetEntry(mtl);
    if(!mtl.memset->empty() && ((mtl.memset->back().getEnd() - mtl.startInstance + 1) >= mtl.numInstancesRequired)){
      return 1;
    }
  }
  else{
    printf("error, wrong state: %s\n", memorytracetext);
  }
}


. {
}

\n {
}



%%
