
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

#ifndef LOOPTRACESTREAMER_H
#define LOOPTRACESTREAMER_H

#include "BZ2ParserState.h"
//#include "parser_wrappers.h"
#include "LoopTraceCfcLex.h"

class LoopTraceStreamer{
  LoopTraceCfcLex ltcl;
  BZ2ParserState parser;

  int getFullEntry(LoopTraceEntry& lte);

public:
  LoopTraceStreamer();

  /* Returns 1 if a valid LoopTraceEntry was filled, 0 otherwise */
  int getNextEntry(LoopTraceEntry& lte);

  /* Returns a list of instructions and the number of invocations */
  pair<set<uintptr_t>, uint64_t> parseForInstructionList();
};

#endif
