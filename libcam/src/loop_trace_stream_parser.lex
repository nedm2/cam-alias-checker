
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
	#define YY_DECL int looptracestreamlex (LoopTraceEntry& loopEntry, bool firstPass, set<uintptr_t> *instructionList, uint64_t *numInvoc)
	int looptracestreamlex (LoopTraceEntry& loopEntry, bool firstPass, set<uintptr_t> *instructionList, uint64_t *numInvoc);
}

%{
struct LoopTraceStreamLex{
  int state;
  uint64_t invocNum;
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
  LoopTraceStreamLex() : state(0), remaining_invoc_entries(0), 
    remaining_inst_entries(0), group_state(0), start_invoc(0), end_invoc(0),
    currLoopTrace(NULL), currInvocation(NULL), nextOutputMarker(0) {}
};
LoopTraceStreamLex ltsl;

//#define DISPLAY printf("%i: %s\n", ltsl.state, looptracestreamtext);
#define DISPLAY 

int readInstEntry(InvocationGroup& invGroup, bool firstPass, set<uintptr_t> *instructionList) {
  if(ltsl.group_state == 0){
    ltsl.currInstruction = ALPHTONUM(looptracestreamtext);
    if(firstPass)
      instructionList->insert(ltsl.currInstruction);
    ltsl.group_state++;
  }
  else if(ltsl.group_state == 1){
    if(!firstPass){
      ltsl.start_iter = strtoull(looptracestreamtext, NULL, 10);
    }
    ltsl.group_state++;
  }
  else if(ltsl.group_state == 2){
    if(!firstPass){
      ltsl.end_iter = strtoull(looptracestreamtext, NULL, 10);
    }
    ltsl.group_state++;
  }
  else if(ltsl.group_state == 3){
    if(!firstPass){
      ltsl.num_dyn_inst = strtoull(looptracestreamtext, NULL, 10);
      if(invGroup.find(ltsl.currInstruction) == invGroup.end()){
        invGroup[ltsl.currInstruction] = LoopInstrGroup();
      }
      invGroup[ltsl.currInstruction].push_back(LoopInstrRec(ltsl.start_iter, ltsl.end_iter, ltsl.num_dyn_inst));
      uint64_t numDynamicInstancesAddedInSingleInvocation = ltsl.num_dyn_inst*(ltsl.end_iter - ltsl.start_iter + 1);
      invGroup[ltsl.currInstruction].addDynamicInstances(numDynamicInstancesAddedInSingleInvocation);
      //uint64_t numDynamicInstancesAddedInInvocationGroup = ltsl.currInvocation->getNumInvocations()*numDynamicInstancesAddedInSingleInvocation;
      //ltsl.currLoopTrace->addDynamicInstancesToCache(ltsl.currInstruction, numDynamicInstancesAddedInInvocationGroup);
      //ltsl.currInvocation->addDynamicInstancesToCache(ltsl.currInstruction, numDynamicInstancesAddedInInvocationGroup);
      if(ltsl.end_iter + 1 > invGroup.getNumIterations())
        invGroup.setNumIterations(ltsl.end_iter + 1);
    }

    ltsl.group_state=0;
    ltsl.remaining_inst_entries--;
    if(ltsl.remaining_inst_entries == 0){
      ltsl.remaining_invoc_entries--;
      if(ltsl.remaining_invoc_entries == 0){
        ltsl.state = 0;
        return 0;
      }
      else{
        ltsl.state = 2;
        return 1;
      }
    }
  }
  else{
    printf("Bad group state: %s", looptracestreamtext);
    abort();
  }
  return 2;
}

%}

%option noyywrap
%option noinput
%option nounput
%option header-file="loop_trace_stream_parser.h"
%option outfile="loop_trace_stream_parser.cpp"
%option prefix="looptracestream"

NUMBER	[0-9]+
CLOSEBRACE \}
DASH \-
LOOPNAME [a-zA-Z][^ \t\n\[\]]+

%%

{NUMBER} {
  if(ltsl.state == 0){
    //if(!firstPass){
    //  uintptr_t loopid = ALPHTONUM(looptracestreamtext);
    //  loops[loopid] = StaticLoopRec();
    //  ltsl.currLoopTrace = &loops[loopid];
    //}
    ltsl.state++;
  }
  else if(ltsl.state == 1){
    DISPLAY;
    ltsl.remaining_invoc_entries = strtoull(looptracestreamtext, NULL, 10);
    if(ltsl.remaining_invoc_entries > 0){
      ltsl.state++;
    }
    else {
      ltsl.state = 0;
    }
  }
  else if(ltsl.state == 2){
    DISPLAY;
    ltsl.invocNum = strtoull(looptracestreamtext, NULL, 10);
    if(firstPass && ltsl.invocNum > *numInvoc)
      *numInvoc = ltsl.invocNum;
    ltsl.state++;
  }
  else if(ltsl.state == 3){
    DISPLAY;
    printf("Can't process a number in state 3: %s\n", looptracestreamtext);
    abort();
  }
  else if(ltsl.state == 4){
    DISPLAY;
    uint64_t invocNum = strtoull(looptracestreamtext, NULL, 10);
    if(firstPass && invocNum > *numInvoc)
      *numInvoc = invocNum;
    loopEntry.first.push_back(pair<uint64_t, uint64_t>(ltsl.invocNum, invocNum));
    ltsl.state = 2;
  }
  
  //else if(ltsl.state == 5){
  //  if(!firstPass){
  //    ltsl.end_invoc = strtoull(looptracestreamtext, NULL, 10);
  //    ltsl.currLoopTrace->push_back(InvocationGroup(ltsl.start_invoc, ltsl.end_invoc));
  //    ltsl.currInvocation = &ltsl.currLoopTrace->back();
  //    ltsl.currLoopTrace->setNumInvocations(ltsl.end_invoc + 1);
  //  }
  //  ltsl.state++;
  //}
  else if(ltsl.state == 5){
    DISPLAY;
    ltsl.remaining_inst_entries = strtoull(looptracestreamtext, NULL, 10);
    if(ltsl.remaining_inst_entries > 0){
      ltsl.group_state = 0;
      ltsl.state++;
    }
    else{ 
      ltsl.remaining_invoc_entries--;
      if(ltsl.remaining_invoc_entries == 0){
        ltsl.state = 0;
        return 0;
      }
      else{
        ltsl.state = 2;
        return 1;
      }
    }
  }
  else if(ltsl.state == 6){
    DISPLAY;
    int ret = readInstEntry(loopEntry.second, firstPass, instructionList);
    if(ret != 2)
      return ret;
  }
  else{
    fprintf(stderr, "Loop trace parser error, wrong state: %s\n", looptracestreamtext);
    abort();
  }
}

{DASH} {
  ltsl.state++;
}

, {
  DISPLAY;
  if(ltsl.state == 3){
    loopEntry.first.push_back(pair<uint64_t, uint64_t>(ltsl.invocNum, ltsl.invocNum));
    ltsl.state = 2;
  }
}

{CLOSEBRACE} {
  DISPLAY;
  if(ltsl.state == 3){
    loopEntry.first.push_back(pair<uint64_t, uint64_t>(ltsl.invocNum, ltsl.invocNum));
  }
  ltsl.state = 5;
}


. {
}

\n {
}



%%
