
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

#ifndef REPETITIONPATTERN_H
#define REPETITIONPATTERN_H

#include <vector>
#include <iostream>
#include <set>
using namespace std;

class RepetitionPattern{
  vector<pair<uint64_t, uint64_t>> pattern;

public:
  typedef vector<pair<uint64_t, uint64_t>>::iterator iterator;
  typedef vector<pair<uint64_t, uint64_t>>::const_iterator const_iterator;
  void addPair(uint64_t start, uint64_t end);
  iterator begin();
  iterator end();
  const_iterator begin() const;
  const_iterator end() const;
  uint64_t numRanges();
  void clear();
  bool empty();
  void removeFirstInstance();
  void removeLastInstance();
  uint64_t getFirstInstance();
  uint64_t getLastInstance();
  bool isSingleton();
  bool overlapsWith(RepetitionPattern& other);
  void deleteFirstInstance();
  void deleteLastInstance();
  friend ostream& operator<<(ostream& os, const RepetitionPattern& rp);
};

template <class Data> class RepetitionPatternLookup;
template <class Data> ostream& operator<< (ostream& os, const RepetitionPatternLookup<Data>& rp);

template <class Data> 
class RepetitionPatternLookup{
  vector<tuple<uint64_t, uint64_t, Data*>> lut;
public:
  void buildLut(vector<pair<RepetitionPattern&, Data*>>& d);
  void clearLut();
  typedef pair<typename vector<tuple<uint64_t, uint64_t, Data*>>::iterator, uint64_t> iterator;
  iterator begin();
  iterator end();
  iterator next(iterator iter);
  iterator nextEntry(iterator iter);
  Data* getDataFromII(iterator iter);
  uint64_t getStartFromII(iterator iter);
  uint64_t getEndFromII(iterator iter);
  uint64_t getRangeNumFromII(iterator iter);
  Data* lookupData(uint64_t n);

  uint64_t getLastInstance();

  friend ostream& operator<< <> (ostream& os, const RepetitionPatternLookup<Data>& rp);
};

template <class Data> class RepetitionPatternSortedLookup;
template <class Data> ostream& operator<< (ostream& os, const RepetitionPatternSortedLookup<Data>& rp);

template <class Data> 
class RepetitionPatternSortedLookup{
  struct LutCompare {
    bool operator() (const tuple<uint64_t, uint64_t, Data>& t1, const tuple<uint64_t, uint64_t, Data>& t2){
      return get<1>(t1) < get<1>(t2);
    }
  };

  set<tuple<uint64_t, uint64_t, Data>, LutCompare> lut;

public:
  //void buildLut(vector<pair<RepetitionPattern&, Data*>>& d);
  void insertToLut(const RepetitionPattern& rp, Data d);
  void clearLut();
  typedef typename set<tuple<uint64_t, uint64_t, Data>, LutCompare>::iterator iterator;
  //iterator begin();
  iterator end();
  //iterator next(iterator iter);
  //iterator nextEntry(iterator iter);
  //Data* getDataFromII(iterator iter);
  //uint64_t getStartFromII(iterator iter);
  //uint64_t getEndFromII(iterator iter);
  //uint64_t getRangeNumFromII(iterator iter);
  iterator lookupData(uint64_t n);
  Data getDataFromIterator(iterator i);


  friend ostream& operator<< <> (ostream& os, const RepetitionPatternSortedLookup<Data>& rp);
};

#endif
