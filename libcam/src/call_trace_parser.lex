
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
  #include "CallTrace.h"
  #include "cam_system.h"
  using namespace CallTraceAnalysis;

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int calltracelex (CallTraces& trace, bool firstPass, set<uintptr_t> *callList, set<uintptr_t> *subInstrList)
	int calltracelex (CallTraces& trace, bool firstPass, set<uintptr_t> *callList, set<uintptr_t> *subInstrList);
}

%{
struct CallTraceLex{
  int state;
  uint64_t remaining_call_entries;
  uint64_t remaining_inst_entries;
  uint64_t startCallInvoc;
  uint64_t endCallInvoc;
  uintptr_t subInstrID;
  CallTrace *currCallTrace;
  CallTraceRec *currCallTraceRec;
  CallTraceLex() : state(0), remaining_call_entries(0), 
    remaining_inst_entries(0), startCallInvoc(0), endCallInvoc(0),
    currCallTrace(NULL), currCallTraceRec(NULL) {}
};
CallTraceLex ctl;

%}

%option noyywrap
%option noinput
%option nounput
%option header-file="call_trace_parser.h"
%option outfile="call_trace_parser.cpp"
%option prefix="calltrace"

NUMBER	[0-9]+
LOOPNAME [a-zA-Z][^ \t\n\[\]]+

%%

{NUMBER} {
  if(ctl.state == 0){
    uintptr_t callid = ALPHTONUM(calltracetext);
    if(!firstPass){
      trace[callid] = CallTrace();
      ctl.currCallTrace = &trace[callid];
    }
    else
      callList->insert(callid);
    ctl.state++;
  }
  else if(ctl.state == 1){
    ctl.remaining_call_entries = strtoull(calltracetext, NULL, 10);
    if(ctl.remaining_call_entries > 0){
      ctl.state++;
    }
    else {
      ctl.state = 0;
    }
  }
  else if(ctl.state == 2){
    if(!firstPass)
      ctl.startCallInvoc = strtoull(calltracetext, NULL, 10);
    ctl.state++;
  }
  else if(ctl.state == 3){
    if(!firstPass)
      ctl.endCallInvoc = strtoull(calltracetext, NULL, 10);
    ctl.state++;
  }
  else if(ctl.state == 4){
    ctl.remaining_inst_entries = strtoull(calltracetext, NULL, 10);
    if(!firstPass){
      ctl.currCallTrace->push_back(CallTraceRec(ctl.startCallInvoc, ctl.endCallInvoc));
      ctl.currCallTraceRec = &ctl.currCallTrace->back();
    }
    if(ctl.remaining_inst_entries > 0){
      ctl.state++;
    }
    else{ 
      ctl.remaining_call_entries--;
      if(ctl.remaining_call_entries == 0)
        ctl.state = 0;
      else
        ctl.state = 2;
    }
  }
  else if(ctl.state == 5){
    ctl.subInstrID = ALPHTONUM(calltracetext);
    if(firstPass)
      subInstrList->insert(ctl.subInstrID);
    ctl.state++;
  }
  else if(ctl.state == 6){
    if(!firstPass){
      ctl.currCallTraceRec->push_back(InstrCallRec(ctl.subInstrID, strtoull(calltracetext, NULL, 10)));
      ctl.currCallTrace->addToInstructionSet(ctl.subInstrID);
    }
    ctl.remaining_inst_entries--;
    if(ctl.remaining_inst_entries > 0){
      ctl.state = 5;
    }
    else{
      ctl.remaining_call_entries--;
      if(ctl.remaining_call_entries > 0)
        ctl.state = 2;
      else
        ctl.state = 0;
    }
  }
  else{
    fprintf(stderr, "Call trace parser error, wrong state: %s\n", calltracetext);
    abort();
  }
}


. {
}

\n {
}



%%
