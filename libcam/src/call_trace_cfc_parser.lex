
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
  #include <stack>
  #include "CallTrace.h"
  #include "cam_system.h"
  #include "CallTraceCfcLex.h"
  using namespace std;

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int calltracecfclex (void* state)
	int calltracecfclex (void* state);
}

%{


enum TokenType { NUMBER_TOKEN, COMMA_TOKEN, DASH_TOKEN, CLOSEBRACE_TOKEN, OPENPAREN_TOKEN, CLOSEPAREN_TOKEN };

//#define DISPLAY printf("%i: %s\n", ctcl.state, calltracecfctext);
#define DISPLAY 

void readCfc(CallTraceCfcLex& ctcl, TokenType token) {
  if(ctcl.firstPass){
    if(token == NUMBER_TOKEN){
      if(ctcl.cfc_state == 1)
        ctcl.cfc_state = 0;
      else
        ctcl.subInstrList->insert(ALPHTONUM(calltracecfctext));
    }
    else if(token == COMMA_TOKEN){
      ctcl.cfc_state = 1;
    }
  }
  else{
    if(token == OPENPAREN_TOKEN){
      ctcl.patternStack.push(CompressionPattern());
    }
    else if(token == CLOSEPAREN_TOKEN){
      CompressionPattern cp = ctcl.patternStack.top();
      ctcl.patternStack.pop();
      if(ctcl.patternStack.empty()){
        ctcl.callEntry->addCallCfc(ctcl.currCallID, cp);
      }
      else{
        ctcl.patternStack.top().addPattern(cp);
      }
    }
    else if(token == COMMA_TOKEN){
      ctcl.cfc_state = 1;
    }
    else if(token == NUMBER_TOKEN){
      if(ctcl.cfc_state == 1){
        ctcl.patternStack.top().setNumRepetitions(strtoull(calltracecfctext, NULL, 10));
        ctcl.cfc_state = 0;
      }
      else{
        ctcl.patternStack.top().setSymbol(strtoul(calltracecfctext, NULL, 10));
      }
    }
  }
}

void readRepetitionPattern(CallTraceCfcLex& ctcl, TokenType token){
  if(ctcl.firstPass){
    if(token == CLOSEBRACE_TOKEN){
      ctcl.reppatState = 0;
      ctcl.state = ctcl.state == INVOC_PATTERN ? NUM_CALLS : CFC;
    }
  }
  else{
    if(token == NUMBER_TOKEN){
      /* Invocation number start (single invocation or first of pair) */
      if(ctcl.reppatState == 0){
        DISPLAY;
        ctcl.invocNum = strtoull(calltracecfctext, NULL, 10);
        ctcl.reppatState++;
      }
      /* Inovcation number end, should only get here if there was a range */
      else if(ctcl.reppatState == 2){
        DISPLAY;
        uint64_t invocNum = strtoull(calltracecfctext, NULL, 10);
        if(ctcl.state == INVOC_PATTERN)
          ctcl.callEntry->addInvocationPattern(ctcl.invocNum, invocNum);
        else
          ctcl.callEntry->addCallPattern(ctcl.currCallID, ctcl.invocNum, invocNum);
        ctcl.reppatState = 0;
      }
    }
    else if(token == DASH_TOKEN){ // dash
      ctcl.reppatState++;
    }
    else if(token == COMMA_TOKEN){ // comma
      if(ctcl.reppatState == 1){
        if(ctcl.state == INVOC_PATTERN)
          ctcl.callEntry->addInvocationPattern(ctcl.invocNum, ctcl.invocNum);
        else
          ctcl.callEntry->addCallPattern(ctcl.currCallID, ctcl.invocNum, ctcl.invocNum);
      }
      ctcl.reppatState = 0;
    }
    else if(token == CLOSEBRACE_TOKEN){
      if(ctcl.reppatState == 1){
        if(ctcl.state == INVOC_PATTERN)
          ctcl.callEntry->addInvocationPattern(ctcl.invocNum, ctcl.invocNum);
        else
          ctcl.callEntry->addCallPattern(ctcl.currCallID, ctcl.invocNum, ctcl.invocNum);
      }
      ctcl.reppatState = 0;
      ctcl.state = ctcl.state == INVOC_PATTERN ? NUM_CALLS : CFC;
    }
  }
}

int parserExitCode(CallTraceCfcLex& ctcl){
  if(ctcl.remaining_loopinvoc_entries > 0)
    return 2;
  else
    return 1;
}

%}

%option noyywrap
%option noinput
%option nounput
%option header-file="call_trace_cfc_parser.h"
%option outfile="call_trace_cfc_parser.cpp"
%option prefix="calltracecfc"

NUMBER	[0-9]+
CLOSEBRACE \}
DASH \-
LOOPNAME [a-zA-Z][^ \t\n\[\]]+
OPENPAREN \(
CLOSEPAREN \)
COMMA ,
INVOC_COMPLETE_SYMBOL COMPLETE
INVOC_INCOMPLETE_SYMBOL INCOMPLETE

%%

{NUMBER} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);

  DISPLAY;
  if(ctcl.state == NUM_INVOC){
    ctcl.remaining_loopinvoc_entries = strtoull(calltracecfctext, NULL, 10);
    ctcl.state = INVOC_PATTERN;
  }
  else if(ctcl.state == INVOC_PATTERN){
    readRepetitionPattern(ctcl, NUMBER_TOKEN);
  }
  else if(ctcl.state == NUM_CALLS){
    ctcl.remaining_call_entries = strtoull(calltracecfctext, NULL, 10);
    if(ctcl.remaining_call_entries > 0){
      ctcl.state = CALLID;
    }
    else{
      ctcl.remaining_loopinvoc_entries--;
      ctcl.state = DUMPEND;
    }
  }
  else if(ctcl.state == CALLID){
    ctcl.currCallID = ALPHTONUM(calltracecfctext);
    if(!ctcl.firstPass){
      ctcl.callEntry->addNewCall(ctcl.currCallID);
    }
    else
      ctcl.callList->insert(ctcl.currCallID);
    ctcl.state = NUM_INSTANCES;
  }
  else if(ctcl.state == NUM_INSTANCES){
    ctcl.remaining_callinstance_entries = strtoull(calltracecfctext, NULL, 10);
    if(ctcl.remaining_callinstance_entries > 0){
      if(!ctcl.firstPass)
        ctcl.callEntry->addCallInstance(ctcl.currCallID);
      ctcl.reppatState = 0;
      ctcl.state = INSTANCE_PATTERN;
    }
    else {
      ctcl.remaining_loopinvoc_entries--;
      ctcl.state = DUMPEND;
    }
  }
  /* Invocation repetition pattern */
  else if(ctcl.state == INSTANCE_PATTERN){
    readRepetitionPattern(ctcl, NUMBER_TOKEN);
  }
  /* Iteration CFC */
  else if(ctcl.state == CFC){
    readCfc(ctcl, NUMBER_TOKEN);
  }
  else{
    fprintf(stderr, "Call trace parser error, wrong state: %s\n", calltracecfctext);
    abort();
  }
}

{DASH} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);

  if(ctcl.state == INVOC_PATTERN || ctcl.state == INSTANCE_PATTERN)
    readRepetitionPattern(ctcl, DASH_TOKEN);
}

{COMMA} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);

  DISPLAY;
  if(ctcl.state == INVOC_PATTERN || ctcl.state == INSTANCE_PATTERN)
    readRepetitionPattern(ctcl, COMMA_TOKEN);
  else if(ctcl.state == CFC)
    readCfc(ctcl, COMMA_TOKEN);
}

{CLOSEBRACE} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);

  DISPLAY;
  if(ctcl.state == INVOC_PATTERN || ctcl.state == INSTANCE_PATTERN)
    readRepetitionPattern(ctcl, CLOSEBRACE_TOKEN);
}

{OPENPAREN} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);

  DISPLAY;
  if(ctcl.state == CFC)
    readCfc(ctcl, OPENPAREN_TOKEN);
}

{CLOSEPAREN} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);

  DISPLAY;
  if(ctcl.state == CFC)
    readCfc(ctcl, CLOSEPAREN_TOKEN);
}


. {
}

\n {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);

  // CFC is complete
  if(ctcl.state == CFC){
    ctcl.remaining_callinstance_entries--;
    if(ctcl.remaining_callinstance_entries > 0){
      if(!ctcl.firstPass)
        ctcl.callEntry->addCallInstance(ctcl.currCallID);
      ctcl.state = INSTANCE_PATTERN;
    }
    else{
      ctcl.remaining_call_entries--;
      if(ctcl.remaining_call_entries > 0){
        ctcl.state = CALLID;
      }
      else{
        ctcl.remaining_loopinvoc_entries--;
        ctcl.state = DUMPEND;
      }
    }
  }
}

{INVOC_COMPLETE_SYMBOL} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);
  if(ctcl.state != DUMPEND){ cerr << "Call trace found completion symbol in wrong state\n"; abort(); }

  if(ctcl.remaining_loopinvoc_entries > 0)
    ctcl.state = INVOC_PATTERN;
  else
    ctcl.state = NUM_INVOC;
  ctcl.rstatus = ctcl.INVOC_COMPLETE;
  return 1;
}

{INVOC_INCOMPLETE_SYMBOL} {
  CallTraceCfcLex& ctcl = *(CallTraceCfcLex*)(state);
  if(ctcl.state != DUMPEND){ cerr << "Call trace found completion symbol in wrong state\n"; abort(); }

  if(ctcl.remaining_loopinvoc_entries > 0)
    ctcl.state = INVOC_PATTERN;
  else
    ctcl.state = NUM_INVOC;
  ctcl.rstatus = ctcl.INVOC_INCOMPLETE;
  return 1;
}




%%
