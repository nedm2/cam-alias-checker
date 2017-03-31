
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

#include "LoopTraceStreamer.h"
#include "loop_trace_cfc_parser.h"
#include <cassert>

LoopTraceStreamer::LoopTraceStreamer() 
  : ltcl(LoopTraceCfcLex())
  , parser(BZ2ParserState(
        "loop_trace.txt.bz2"
      , looptracecfclex
      , looptracecfc_scan_buffer
      , looptracecfc_switch_to_buffer
      , looptracecfc_delete_buffer
      , &ltcl
    ))
{ 
}

int LoopTraceStreamer::getFullEntry(LoopTraceEntry& lte){
  assert(ltcl.state == 0 || ltcl.state == 2);
  ltcl.loopEntry = &lte;
  ltcl.rstatus = ltcl.ACTIVE;
  ltcl.firstPass = false;
  while(1){
    int dataRemaining = parser.parseBlock();
    if(!dataRemaining)
      return 0;
    if(!ltcl.rstatus == ltcl.ACTIVE)
      break;
  }
  return 1;
}

int LoopTraceStreamer::getNextEntry(LoopTraceEntry& lte){
  if(!getFullEntry(lte))
    return 0;

  while(ltcl.rstatus == ltcl.INVOC_INCOMPLETE){
    LoopTraceEntry l;
    getFullEntry(l);
    if(!lte.lastAndFirstInvocationOverlap(l)) abort();
    if(!lte.mergeIntoAndReturnRemaining(l).empty()) abort();
  }

  return 1;
}

pair<set<uintptr_t>, uint64_t> LoopTraceStreamer::parseForInstructionList(){
  set<uintptr_t> instructionList;
  uint64_t numInvoc = 0;
  LoopTraceEntry lte;

  ltcl.loopEntry = &lte;
  ltcl.instructionList = &instructionList;
  ltcl.numInvoc = &numInvoc;
  ltcl.firstPass = true;

  while(parser.parseBlock()){}
  return pair<set<uintptr_t>, uint64_t>(instructionList, numInvoc+1);
}
