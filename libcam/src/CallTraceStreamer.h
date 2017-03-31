
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

#ifndef CALLTRACESTREAMER_H
#define CALLTRACESTREAMER_H

#include "BZ2ParserState.h"
#include "CallTraceCfcLex.h"

class CallTraceStreamer{
  CallTraceCfcLex ctcl;
  BZ2ParserState parser;

  int getFullEntry(CallTraceLoopInvocationGroup& clig);

public:
  CallTraceStreamer();

  /* Returns 1 if a valid CallTraceLoopInvocationGroup was filled, 0 otherwise */
  int getNextEntry(CallTraceLoopInvocationGroup& clig);

  /* Returns lists of call ids and sub-instruction ids */
  pair<set<uintptr_t>, set<uintptr_t>> parseForInstructionLists();
};

#endif
