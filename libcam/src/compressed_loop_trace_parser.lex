
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
  #include <map>
  #include <vector>
  using namespace std;

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int xlooptracelex (map<pair<uint64_t, uint64_t>, char *>& t, uint64_t& loopid, uint64_t& numlines)
	int xlooptracelex (map<pair<uint64_t, uint64_t>, char *>& t, uint64_t& loopid, uint64_t& numlines);
}

%{
struct CompressedLoopTraceLex{
  int state;
  uint64_t num;
  vector<pair<uint64_t, uint64_t>> pattern;
  CompressedLoopTraceLex() : state(0)  {}
};
CompressedLoopTraceLex cltl;


%}

%option noyywrap
%option noinput
%option nounput
%option header-file="compressed_loop_trace_parser.h"
%option outfile="compressed_loop_trace_parser.cpp"
%option prefix="xlooptrace"

CLOSEBRACE \}
NUMBER	[0-9]+
LOOPNAME [a-zA-Z][^ \t\n\[\]]+

%%

{NUMBER} {
  if(cltl.state == 0){
    loopid = strtoull(xlooptracetext, NULL, 10);
    cltl.state++;
  }
  else if(cltl.state == 1){
    numlines = strtoull(xlooptracetext, NULL, 10);
    cltl.state++;
  }
  else if(cltl.state == 2){
    cltl.num = strtoull(xlooptracetext, NULL, 10);
    cltl.state++;
  }
  else if(cltl.state == 4){
    cltl.pattern.push_back(pair<uint64_t, uint64_t>(cltl.num, strtoull(xlooptracetext, NULL, 10)));
    cltl.state = 2;
  }
  else{
    printf("Can't process a number in state %i: %s\n", cltl.state, xlooptracetext);
    abort();
  }
    
}

\- {
  cltl.state++;
}

,|{CLOSEBRACE} {
  if(cltl.state == 3){
    cltl.pattern.push_back(pair<uint64_t, uint64_t>(cltl.num, cltl.num));
    cltl.state = 2;
  }
}

({NUMBER}[ ]*\[.*\]\n)|0[ ]*\n {
  char *newPattern = (char *)malloc(sizeof(char)*(strlen(xlooptracetext)+1));
  strcpy(newPattern, xlooptracetext);
  for(auto i = cltl.pattern.begin(); i != cltl.pattern.end(); i++){
    t[*i] = newPattern;
  }
  cltl.pattern.clear();
  cltl.state = 2;
}

. {
}



%%
