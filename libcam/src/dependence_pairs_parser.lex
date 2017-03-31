
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
  #include <set>
  using namespace std;

  #ifdef yytext_ptr
    #undef yytext_ptr
  #endif
  #ifdef YY_DECL
    #undef YY_DECL
  #endif
	#define YY_DECL int deppairslex (set<pair<uintptr_t, uintptr_t>>& rw_pairs, set<pair<uintptr_t, uintptr_t>>& ww_pairs)
	int deppairslex (set<pair<uintptr_t, uintptr_t>>& rw_pairs, set<pair<uintptr_t, uintptr_t>>& ww_pairs);
}

%{
struct DependencePairsLex{
  int state;
  uintptr_t firstEntry;
  uint64_t numEntries;
  DependencePairsLex() : state(0) {}
};
DependencePairsLex dpl;

void readPair(set<pair<uintptr_t, uintptr_t>>& pairs) {
  if(dpl.state == 1 || dpl.state == 4){
    dpl.firstEntry = ALPHTONUM(deppairstext);
    dpl.state++;
  }
  else if(dpl.state == 2 || dpl.state == 5){
    pairs.insert(pair<uintptr_t, uintptr_t>(dpl.firstEntry, ALPHTONUM(deppairstext)));
    dpl.numEntries--;
    if (dpl.numEntries == 0)
      dpl.state++;
    else 
      dpl.state--;
  }
  else
    fprintf(stderr, "Dep pair parser error\n");
}

%}

%option noyywrap
%option noinput
%option nounput
%option header-file="dependence_pairs_parser.h"
%option outfile="dependence_pairs_parser.cpp"
%option prefix="deppairs"

NUMBER	[0-9]+
LOOPNAME [a-zA-Z][^ \t\n\[\]]+

%%

{NUMBER} {
  if(dpl.state == 0){
    dpl.numEntries = ALPHTONUM(deppairstext);
    if(dpl.numEntries > 0)
      dpl.state++;
    else
      dpl.state = 3;
  }
  else if(dpl.state == 1 || dpl.state == 2){
    readPair(rw_pairs);
  }
  else if(dpl.state == 3){
    dpl.numEntries = ALPHTONUM(deppairstext);
    if(dpl.numEntries > 0)
      dpl.state++;
    else
      dpl.state = 6;
  }
  else if(dpl.state == 4 || dpl.state == 5){
    readPair(ww_pairs);
  }
  else{
    fprintf(stderr, "Dependence pairs parser error: %s\n", deppairstext);
    abort();
  }
}


. {
}

\n {
}



%%
