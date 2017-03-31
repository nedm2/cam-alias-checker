
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

#ifndef CONTROLFLOWCOMPRESSOR_H
#define CONTROLFLOWCOMPRESSOR_H

#include <deque>
#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include "cam_system.h"
#include "memory_allocator.hh"
using namespace std;

typedef uintptr_t tracer_symbol;

class TracerCompressionPattern{
  CamMemoryAllocator* allocator;
  vector<TracerCompressionPattern*> pattern;
  uint64_t numRepetitions;
  tracer_symbol sym;

public:
  friend ostream& operator<<(ostream& os, const TracerCompressionPattern& p);
  bool operator==(const TracerCompressionPattern& other) const ;
  bool operator!=(const TracerCompressionPattern& other) const ;
  TracerCompressionPattern(CamMemoryAllocator* alloc, vector<TracerCompressionPattern*> pattern_p, uint64_t numRepetitions_p);
  TracerCompressionPattern(CamMemoryAllocator* alloc, const TracerCompressionPattern& other);
  TracerCompressionPattern(CamMemoryAllocator* alloc, tracer_symbol sym_p);
  TracerCompressionPattern(CamMemoryAllocator* alloc);
  TracerCompressionPattern& operator=(const TracerCompressionPattern& other);
  ~TracerCompressionPattern();
  bool patternMatchesSymbols(deque<TracerCompressionPattern*> &symbols);
  void incRepetitions();
  unsigned int getPatternSize();
  vector<tracer_symbol> decompress();
  void addPattern(TracerCompressionPattern& cp);
  void setNumRepetitions(uint64_t n);
  void setSymbol(tracer_symbol s);
  void coutThis();
  map<tracer_symbol, uint64_t> getNumberOfInstancesMap();
};

/*
 * Takes a sequence of symbols and attempts to compress them by recognising repeated patterns.
 * For example
 * (a)(b)(c)(c)(a)(b)(c)(c)(a)(b)(c)(c) --> ((a)(b)(c,2),3)
 *
 * The compressed data is stored in two data structures: 
 *   window - a bounded size queue on which the compression operation are run
 *   storedPatterns - an unbounded size array where data is moved once it has overflown the window
 * 
 * There are two compression operations: 
 *   match - 
 *     Attempt to match a sequence of patterns to a previously established pattern, for example
 *     ((a)(b)(c,2),10)(a)(b)(c,2) --> ((a)(b)(c,2),11)
 *   merge - 
 *     Attempt to merge two identical consecutive sequences of patterns, for example
 *     (a)(b)(c,2)(a)(b)(c,2) --> ((a)(b)(c,2),2)
 * 
 * Every time a symbol is added using insertSymbol, these operations are run repeatedly until 
 * no further compression of the window is possible. It is necessary to run the operations
 * repeatedly so that arbitrarily nested patterns can be recognised.
*/
class ControlFlowCompressor{
  deque<TracerCompressionPattern*> window;
  vector<TracerCompressionPattern*> storedPatterns;
  CamMemoryAllocator* allocator;
  unsigned int maxWindowLength;


public:
  /* Iterate over all stored patterns, first storedPatterns, then window, the int indicates which data structure we're currently on */
  typedef tuple<vector<TracerCompressionPattern*>::const_iterator, deque<TracerCompressionPattern*>::const_reverse_iterator, int> CFCIterator;
  CFCIterator iteratorBegin() const ;
  CFCIterator iteratorNext(CFCIterator iter) const ;
  CFCIterator iteratorEnd() const ;
  const TracerCompressionPattern *iteratorData(CFCIterator iter) const ;

  ControlFlowCompressor(CamMemoryAllocator* alloc);
  ControlFlowCompressor(CamMemoryAllocator* alloc, unsigned int maxWindowLength_p);
  ~ControlFlowCompressor();
  friend ostream& operator<<(ostream& os, const ControlFlowCompressor& cfc);
  bool operator==(const ControlFlowCompressor& other);
  bool operator!=(const ControlFlowCompressor& other);
  void rawOutput();
  void coutThis();

  /*
   * Add a new symbol and run the compression algorithm
  */
  void insertSymbol(tracer_symbol symbol);

  /* 
   * Attempt to match a sequence of patterns to a previously established pattern, for example
   * ((a)(b)(c,2),10)(a)(b)(c,2) --> ((a)(b)(c,2),11)
   * Returns true if a compression was found
  */
  bool match();

  /* 
   * Attempt to merge two identical consecutive sequences of patterns, for example
   * (a)(b)(c,2)(a)(b)(c,2) --> ((a)(b)(c,2),2)
   * Returns true if a compression was found
  */
  bool merge();
  bool isRepeatedSequence(unsigned int width);
  void clear();
  bool isEmpty();
  vector<tracer_symbol> decompress();
};

#endif
