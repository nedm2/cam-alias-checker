
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
  #include <set>
  #include <static_inst_rec.h>
  #include "StaticLoopRec.h"
  #include "cam_system.h"
  using namespace std;

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int looptracelex (LoopTraces& loops, bool firstPass, set<uintptr_t> *instructionList)
	int looptracelex (LoopTraces& loops, bool firstPass, set<uintptr_t> *instructionList);
}

%{
struct InstructionTraceLex{
  int state;
  uint64_t remaining_invoc_entries;
  uint64_t remaining_inst_entries;
  int group_state;
  uint64_t start_invoc;
  uint64_t end_invoc;
  uintptr_t currInstruction;
  uint64_t start_iter;
  uint64_t end_iter;
  uint64_t num_dyn_inst;
  StaticLoopRec *currLoopTrace;
  InvocationGroup *currInvocation;
  uint64_t nextOutputMarker;
  InstructionTraceLex() : state(0), remaining_invoc_entries(0), 
    remaining_inst_entries(0), group_state(0), start_invoc(0), end_invoc(0),
    currLoopTrace(NULL), currInvocation(NULL), nextOutputMarker(0) {}
};
InstructionTraceLex itl;

void readInstEntry(bool firstPass, set<uintptr_t> *instructionList) {
  if(itl.group_state == 0){
    itl.currInstruction = ALPHTONUM(looptracetext);
    if(firstPass)
      instructionList->insert(itl.currInstruction);
    itl.group_state++;
  }
  else if(itl.group_state == 1){
    if(!firstPass){
      itl.start_iter = strtoull(looptracetext, NULL, 10);
    }
    itl.group_state++;
  }
  else if(itl.group_state == 2){
    if(!firstPass){
      itl.end_iter = strtoull(looptracetext, NULL, 10);
    }
    itl.group_state++;
  }
  else if(itl.group_state == 3){
    if(!firstPass){
      itl.num_dyn_inst = strtoull(looptracetext, NULL, 10);
      if(itl.currInvocation && itl.currInvocation->find(itl.currInstruction) == itl.currInvocation->end()){
        (*(itl.currInvocation))[itl.currInstruction] = LoopInstrGroup(itl.currLoopTrace->getTotalNumberOfDynamicInstances(itl.currInstruction));
      }
      (*(itl.currInvocation))[itl.currInstruction].push_back(LoopInstrRec(itl.start_iter, itl.end_iter, itl.num_dyn_inst));
      uint64_t numDynamicInstancesAddedInSingleInvocation = itl.num_dyn_inst*(itl.end_iter - itl.start_iter + 1);
      uint64_t numDynamicInstancesAddedInInvocationGroup = itl.currInvocation->getNumInvocations()*numDynamicInstancesAddedInSingleInvocation;
      itl.currLoopTrace->addDynamicInstancesToCache(itl.currInstruction, numDynamicInstancesAddedInInvocationGroup);
      itl.currInvocation->addDynamicInstancesToCache(itl.currInstruction, numDynamicInstancesAddedInInvocationGroup);
      (*(itl.currInvocation))[itl.currInstruction].addDynamicInstances(numDynamicInstancesAddedInSingleInvocation);
      if(itl.end_iter + 1 > itl.currInvocation->getNumIterations())
        itl.currInvocation->setNumIterations(itl.end_iter + 1);
    }

    itl.group_state=0;
    itl.remaining_inst_entries--;
    if(itl.remaining_inst_entries == 0){
      itl.remaining_invoc_entries--;
      if(itl.remaining_invoc_entries == 0)
        itl.state = 0;
      else
        itl.state = 2;
    }
  }
}

%}

%option noyywrap
%option noinput
%option nounput
%option header-file="loop_trace_parser.h"
%option outfile="loop_trace_parser.cpp"
%option prefix="looptrace"

NUMBER	[0-9]+
LOOPNAME [a-zA-Z][^ \t\n\[\]]+

%%

{NUMBER} {
  if(itl.state == 0){
    if(!firstPass){
      uintptr_t loopid = ALPHTONUM(looptracetext);
      loops[loopid] = StaticLoopRec();
      itl.currLoopTrace = &loops[loopid];
    }
    itl.state++;
  }
  else if(itl.state == 1){
    itl.remaining_invoc_entries = strtoull(looptracetext, NULL, 10);
    if(itl.remaining_invoc_entries > 0){
      itl.state++;
    }
    else {
      itl.state = 0;
    }
  }
  else if(itl.state == 2){
    if(!firstPass){
      itl.start_invoc = strtoull(looptracetext, NULL, 10);
    }
    itl.state++;
  }
  else if(itl.state == 3){
    if(!firstPass){
      itl.end_invoc = strtoull(looptracetext, NULL, 10);
      itl.currLoopTrace->push_back(InvocationGroup(itl.start_invoc, itl.end_invoc));
      itl.currInvocation = &itl.currLoopTrace->back();
      itl.currLoopTrace->setNumInvocations(itl.end_invoc + 1);
    }
    itl.state++;
  }
  else if(itl.state == 4){
    itl.remaining_inst_entries = strtoull(looptracetext, NULL, 10);
    if(itl.remaining_inst_entries > 0){
      itl.group_state = 0;
      itl.state++;
    }
    else{ 
      itl.remaining_invoc_entries--;
      if(itl.remaining_invoc_entries == 0)
        itl.state = 0;
      else
        itl.state = 2;
    }
  }
  else if(itl.state == 5){
    readInstEntry(firstPass, instructionList);
  }
  else{
    fprintf(stderr, "Loop trace parser error, wrong state: %s\n", looptracetext);
    abort();
  }
}


. {
}

\n {
}



%%
