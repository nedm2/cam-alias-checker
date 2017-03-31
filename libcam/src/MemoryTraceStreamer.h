
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

#ifndef MEMORYTRACESTREAMER_H
#define MEMORYTRACESTREAMER_H

#include <map>
#include <vector>
#include "static_inst_rec.h"
#include "parser_wrappers.h"
#include "BZ2ParserState.h"
#include "memory_trace_parser.h"

class MemoryTraceStreamer{
  uintptr_t instrID;
  MemSet remainder;
  MemoryTraceLex mtl;
  BZ2ParserState parser;
  bool write;
  uint64_t lastReturnedInstance;
  int status;

  uint64_t getNumBufferedInstances(MemSet& memset);

public:
  MemoryTraceStreamer(uintptr_t _instrID, string filename, bool _write);

  /* Parse enough of the trace to get the specified number of instances */ 
  MemSet getNextChunk(uint64_t numInstances, bool allowOvershoot=false);

  uintptr_t getID();

  bool isWrite();

};
#endif
