
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

#ifndef COMPRESSIONPATTERN_H
#define COMPRESSIONPATTERN_H

#include <iostream>
#include <vector>
#include <deque>
#include <map>
using namespace std;

typedef uintptr_t c_symbol;

class CompressionPattern;

class CompressionPatternChild{
  vector<CompressionPattern> pattern;
  uint64_t numRepetitions;

public:
  friend ostream& operator<<(ostream& os, const CompressionPatternChild& p);
  bool operator==(const CompressionPatternChild& other) const ;
  CompressionPatternChild();
  void push_back(CompressionPattern cp);
  void setNumRepetitions(uint64_t n);
  void incRepetitions();
  unsigned int getPatternSize();
  map<c_symbol, uint64_t> getNumberOfInstancesMap();
};

class CompressionPattern{
  c_symbol sym;
  CompressionPatternChild* child;

public:
  friend ostream& operator<<(ostream& os, const CompressionPattern& p);
  bool operator==(const CompressionPattern& other) const ;
  bool operator!=(const CompressionPattern& other) const ;
  //CompressionPattern(vector<CompressionPattern*> pattern_p, uint64_t numRepetitions_p);
  CompressionPattern(const CompressionPattern& other);
  CompressionPattern();
  CompressionPattern& operator=(const CompressionPattern& other);
  ~CompressionPattern();
  CompressionPattern(c_symbol sym_p);
  //bool patternMatchesSymbols(deque<CompressionPattern*> &symbols);
  void incRepetitions();
  unsigned int getPatternSize();
  //vector<c_symbol> decompress();
  void addPattern(CompressionPattern& cp);
  void setNumRepetitions(uint64_t n);
  void setSymbol(c_symbol s);
  map<c_symbol, uint64_t> getNumberOfInstancesMap();
  bool hasChild() const;
};

#endif
