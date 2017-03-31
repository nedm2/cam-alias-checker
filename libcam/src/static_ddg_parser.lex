
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
  #include "cam_system.h"
  #include <vector>
  #include <set>
  #include <tuple>
  using namespace std;

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int staticddglex (vector<tuple<uintptr_t, uintptr_t, unsigned int>>& static_ddg)
	int staticddglex (vector<tuple<uintptr_t, uintptr_t, unsigned int>>& static_ddg);
}

%{
//struct InstructionTraceLex{
//  int state;
//  uint64_t remaining_invoc_entries;
//  uint64_t remaining_inst_entries;
//  int group_state;
//  uint64_t start_invoc;
//  uint64_t end_invoc;
//  uintptr_t currInstruction;
//  uint64_t start_iter;
//  uint64_t end_iter;
//  uint64_t num_dyn_inst;
//  StaticLoopRec *currLoopTrace;
//  InvocationGroup *currInvocation;
//  uint64_t nextOutputMarker;
//  InstructionTraceLex() : state(0), remaining_invoc_entries(0), 
//    remaining_inst_entries(0), group_state(0), start_invoc(0), end_invoc(0),
//    currLoopTrace(NULL), currInvocation(NULL), nextOutputMarker(0) {}
//};
//InstructionTraceLex itl;
struct StaticDDGLex{
  int state;
  uintptr_t firstInst;
  uintptr_t secondInst;
  StaticDDGLex() : state(0) {}
};
StaticDDGLex sdl;


%}

%option noyywrap
%option noinput
%option nounput
%option header-file="static_ddg_parser.h"
%option outfile="static_ddg_parser.cpp"
%option prefix="staticddg"

NUMBER	[0-9]+
LOOPNAME [a-zA-Z][^ \t\n\[\]]+

%%

{NUMBER} {
  if(sdl.state == 0){
    sdl.firstInst = ALPHTONUM(staticddgtext);
    sdl.state++;
  }
  else if(sdl.state == 1){
    sdl.secondInst = ALPHTONUM(staticddgtext);
    sdl.state++;
  }
  else if(sdl.state == 2){
    static_ddg.push_back(tuple<uintptr_t, uintptr_t, unsigned int>(sdl.firstInst, sdl.secondInst, strtoul(staticddgtext, NULL, 10)));
    sdl.state = 0;
  }
  else{
    fprintf(stderr, "Static ddg parser error: %s\n", staticddgtext);
    abort();
  }
}


. {
}

\n {
}



%%
