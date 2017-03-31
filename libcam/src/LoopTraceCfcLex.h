
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

#ifndef LOOPTRACECFCLEX_H
#define LOOPTRACECFCLEX_H

#include <cstdint>
#include <stack>
#include <set>
#include "CompressionPattern.h"
class LoopTraceEntry;

struct LoopTraceCfcLex{
  int state;
  enum ReturnStatus { ACTIVE, INVOC_COMPLETE, INVOC_INCOMPLETE };
  ReturnStatus rstatus;
  int cfc_state;
  int reppatState;
  uint64_t invocNum;
  uint64_t remaining_invoc_entries;
  uint64_t remaining_iter_entries;
  uint64_t start_invoc;
  uint64_t end_invoc;
  uintptr_t currInstruction;
  uint64_t start_iter;
  uint64_t end_iter;
  uint64_t num_dyn_inst;
  stack<CompressionPattern> patternStack;
  LoopTraceEntry *loopEntry;
  bool firstPass;
  set<uintptr_t> *instructionList;
  uint64_t *numInvoc;
  LoopTraceCfcLex() : state(0), rstatus(ACTIVE), cfc_state(0), reppatState(0), remaining_invoc_entries(0), 
    remaining_iter_entries(0), start_invoc(0), end_invoc(0) {}
};

#endif
