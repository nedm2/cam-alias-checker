
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
  //#include <static_inst_rec.h>
  #include "LoopTraceCfcLex.h"
  #include "StaticLoopRec.h"
  #include "CompressionPattern.h"
  #include "cam_system.h"
  using namespace std;

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int looptracecfclex (void *state)
	int looptracecfclex (void *state);
}

%{
enum TokenType { NUMBER_TOKEN, COMMA_TOKEN, DASH_TOKEN, CLOSEBRACE_TOKEN, OPENPAREN_TOKEN, CLOSEPAREN_TOKEN };

//#define DISPLAY printf("%i: %s\n", ltcl.state, looptracecfctext);
#define DISPLAY 

void readCfc(LoopTraceCfcLex& ltcl, TokenType token) {
  if(ltcl.firstPass){
    if(token == NUMBER_TOKEN){
      if(ltcl.cfc_state == 1)
        ltcl.cfc_state = 0;
      else
        ltcl.instructionList->insert(ALPHTONUM(looptracecfctext));
    }
    else if(token == COMMA_TOKEN){
      ltcl.cfc_state = 1;
    }
  }
  else{
    if(token == OPENPAREN_TOKEN){
      ltcl.patternStack.push(CompressionPattern());
    }
    else if(token == CLOSEPAREN_TOKEN){
      CompressionPattern cp = ltcl.patternStack.top();
      ltcl.patternStack.pop();
      if(ltcl.patternStack.empty()){
        ltcl.loopEntry->addCfc(cp);
      }
      else{
        ltcl.patternStack.top().addPattern(cp);
      }
    }
    else if(token == COMMA_TOKEN){
      ltcl.cfc_state = 1;
    }
    else if(token == NUMBER_TOKEN){
      if(ltcl.cfc_state == 1){
        ltcl.patternStack.top().setNumRepetitions(strtoull(looptracecfctext, NULL, 10));
        ltcl.cfc_state = 0;
      }
      else{
        ltcl.patternStack.top().setSymbol(strtoul(looptracecfctext, NULL, 10));
      }
    }
  }
}

void readRepetitionPattern(LoopTraceCfcLex& ltcl, TokenType token){
  if(ltcl.firstPass){
    if(token == CLOSEBRACE_TOKEN){
      ltcl.reppatState = 0;
      ltcl.state++;
    }
    else if(token == NUMBER_TOKEN){
      ltcl.invocNum = strtoull(looptracecfctext, NULL, 10);
      if(ltcl.state == 2 && ltcl.invocNum > *ltcl.numInvoc)
        *ltcl.numInvoc = ltcl.invocNum;
    }
  }
  else{
    if(token == NUMBER_TOKEN){
      /* Invocation number start (single invocation or first of pair) */
      if(ltcl.reppatState == 0){
        DISPLAY;
        ltcl.invocNum = strtoull(looptracecfctext, NULL, 10);
        ltcl.reppatState++;
      }
      /* Inovcation number end, should only get here if there was a range */
      else if(ltcl.reppatState == 2){
        DISPLAY;
        uint64_t invocNum = strtoull(looptracecfctext, NULL, 10);
        if(ltcl.state == 2)
          ltcl.loopEntry->addInvocationPattern(ltcl.invocNum, invocNum);
        else if(ltcl.state == 4)
          ltcl.loopEntry->addIterationPattern(ltcl.invocNum, invocNum);
        ltcl.reppatState = 0;
      }
    }
    else if(token == DASH_TOKEN){ // dash
      ltcl.reppatState++;
    }
    else if(token == COMMA_TOKEN){ // comma
      if(ltcl.reppatState == 1){
        if(ltcl.state == 2)
          ltcl.loopEntry->addInvocationPattern(ltcl.invocNum, ltcl.invocNum);
        else if(ltcl.state == 4)
          ltcl.loopEntry->addIterationPattern(ltcl.invocNum);
      }
      ltcl.reppatState = 0;
    }
    else if(token == CLOSEBRACE_TOKEN){
      if(ltcl.reppatState == 1){
        if(ltcl.state == 2)
          ltcl.loopEntry->addInvocationPattern(ltcl.invocNum, ltcl.invocNum);
        else if(ltcl.state == 4)
          ltcl.loopEntry->addIterationPattern(ltcl.invocNum);
      }
      ltcl.reppatState = 0;
      ltcl.state++;
    }
  }
}

%}

%option noyywrap
%option noinput
%option nounput
%option header-file="loop_trace_cfc_parser.h"
%option outfile="loop_trace_cfc_parser.cpp"
%option prefix="looptracecfc"

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
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);

  /* Loop ID (ignored) */
  if(ltcl.state == 0){
    ltcl.state++;
  }
  /* Number of invocation entries */
  else if(ltcl.state == 1){
    DISPLAY;
    ltcl.remaining_invoc_entries = strtoull(looptracecfctext, NULL, 10);
    if(ltcl.remaining_invoc_entries > 0){
      ltcl.reppatState = 0;
      ltcl.state++;
    }
    else {
      ltcl.state = 0;
    }
  }
  /* Invocation repetition pattern */
  else if(ltcl.state == 2){
    DISPLAY;
    readRepetitionPattern(ltcl, NUMBER_TOKEN);
  }
  /* Number of iteration entries */
  else if(ltcl.state == 3){
    DISPLAY;
    ltcl.remaining_iter_entries = strtoull(looptracecfctext, NULL, 10);
    if(ltcl.remaining_iter_entries > 0){
      if(!ltcl.firstPass)
        ltcl.loopEntry->addIterationGroup();
      ltcl.reppatState = 0;
      ltcl.state++;
    }
    else{ 
      ltcl.remaining_invoc_entries--;
      if(ltcl.remaining_invoc_entries == 0){
        //ltcl.state = 0;
        //ltcl.rstatus = ltcl.END_OF_DUMP;
        //return 1;
        ltcl.state = 6;
      }
      else{
        //ltcl.state = 2;
        //ltcl.rstatus = ltcl.INVOCS_REMAINING;
        //return 2;
        ltcl.state = 6;
      }
    }
  }
  /* Iteration repetition pattern */
  else if(ltcl.state == 4){
    DISPLAY;
    readRepetitionPattern(ltcl, NUMBER_TOKEN);
  }
  /* Iteration CFC */
  else if(ltcl.state == 5){
    readCfc(ltcl, NUMBER_TOKEN);
  }
  else{
    fprintf(stderr, "Loop trace parser error, wrong state: %s\n", looptracecfctext);
    abort();
  }
}

{DASH} {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);

  if(ltcl.state == 2 || ltcl.state == 4)
    readRepetitionPattern(ltcl, DASH_TOKEN);
}

{COMMA} {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);

  DISPLAY;
  if(ltcl.state == 2 || ltcl.state == 4)
    readRepetitionPattern(ltcl, COMMA_TOKEN);
  else if(ltcl.state == 5)
    readCfc(ltcl, COMMA_TOKEN);
}

{CLOSEBRACE} {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);

  DISPLAY;
  if(ltcl.state == 2 || ltcl.state == 4)
    readRepetitionPattern(ltcl, CLOSEBRACE_TOKEN);
}

{OPENPAREN} {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);

  DISPLAY;
  if(ltcl.state == 5)
    readCfc(ltcl, OPENPAREN_TOKEN);
}

{CLOSEPAREN} {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);

  DISPLAY;
  if(ltcl.state == 5)
    readCfc(ltcl, CLOSEPAREN_TOKEN);
}


. {
}

\n {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);

  // CFC is complete
  if(ltcl.state == 5){
    // TODO insert CFC into loop trace entry
    ltcl.remaining_iter_entries--;
    if(ltcl.remaining_iter_entries > 0){
      if(!ltcl.firstPass)
        ltcl.loopEntry->addIterationGroup();
      ltcl.state = 4;
    }
    else{
      ltcl.remaining_invoc_entries--;
      if(ltcl.remaining_invoc_entries > 0){
        //ltcl.state = 2;
        //ltcl.rstatus = ltcl.INVOCS_REMAINING;
        //return 2;
        ltcl.state = 6;
      }
      else{
        //ltcl.state = 0;
        //ltcl.rstatus = ltcl.END_OF_DUMP;
        //return 1;
        ltcl.state = 6;
      }
    }
  }
}

{INVOC_COMPLETE_SYMBOL} {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);
  if(ltcl.state != 6){ abort(); }

  if(ltcl.remaining_invoc_entries > 0)
    ltcl.state = 2;
  else
    ltcl.state = 0;
  ltcl.rstatus = ltcl.INVOC_COMPLETE;
  return 1;
}

{INVOC_INCOMPLETE_SYMBOL} {
  LoopTraceCfcLex& ltcl = *(LoopTraceCfcLex*)(state);
  if(ltcl.state != 6){ abort(); }

  if(ltcl.remaining_invoc_entries > 0)
    ltcl.state = 2;
  else
    ltcl.state = 0;
  ltcl.rstatus = ltcl.INVOC_INCOMPLETE;
  return 1;
}



%%
