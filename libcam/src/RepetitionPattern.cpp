
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

#include <tuple>
#include <algorithm>

#include "RepetitionPattern.h"
#include "CallTrace.h"


void RepetitionPattern::addPair(uint64_t start, uint64_t end){
  pattern.push_back(pair<uint64_t, uint64_t>(start, end));
}

RepetitionPattern::iterator RepetitionPattern::begin(){
  return pattern.begin();
}

RepetitionPattern::iterator RepetitionPattern::end(){
  return pattern.end();
}

RepetitionPattern::const_iterator RepetitionPattern::begin() const{
  return pattern.begin();
}

RepetitionPattern::const_iterator RepetitionPattern::end() const{
  return pattern.end();
}

uint64_t RepetitionPattern::numRanges(){
  return pattern.size();
}

void RepetitionPattern::clear(){
  pattern.clear();
}

bool RepetitionPattern::empty(){
  return pattern.empty();
}

void RepetitionPattern::removeFirstInstance(){
  if(pattern.front().first == pattern.front().second)
    pattern.erase(pattern.begin());
  else
    pattern.front().first++;
}

void RepetitionPattern::removeLastInstance(){
  if(pattern.back().first == pattern.back().second)
    pattern.pop_back();
  else
    pattern.back().second--;
}

uint64_t RepetitionPattern::getFirstInstance(){
  return pattern.front().first;
}

uint64_t RepetitionPattern::getLastInstance(){
  return pattern.back().second;
}

bool RepetitionPattern::isSingleton(){
  return (pattern.size() == 1 && pattern.front().first == pattern.front().second);
}

bool RepetitionPattern::overlapsWith(RepetitionPattern& other){
  auto compare = [] (pair<uint64_t, uint64_t>& r, uint64_t val) { 
    return val < r.second;
  };
  for(auto i = other.pattern.begin(); i != other.pattern.end(); i++){
    auto it = lower_bound(pattern.begin(), pattern.end(), i->first, compare);
    if(it != pattern.end() && i->second > it->first)
      return true;
  }
  return false;
}

void RepetitionPattern::deleteFirstInstance(){
  if(pattern.front().first == pattern.front().second)
    pattern.erase(pattern.begin());
  else
    pattern.front().first++;
}

void RepetitionPattern::deleteLastInstance(){
  if(pattern.back().first == pattern.back().second)
    pattern.pop_back();
  else
    pattern.back().second--;
}

ostream& operator<<(ostream& os, const RepetitionPattern& rp){
  os << "{";
  for(auto i = rp.pattern.begin(); i != rp.pattern.end(); i++){
    os << i->first;
    if(i->first != i->second)
      os << "-" << i->second;
    if(next(i, 1) != rp.pattern.end())
      os << ",";
  }
  os << "}";
  return os;
}


/*
 * RepetitionPatternLookup
*/

template <class Data>
void RepetitionPatternLookup<Data>::buildLut(vector<pair<RepetitionPattern&, Data*>>& d){
#if 0
  typedef vector<tuple<RepetitionPattern::iterator, RepetitionPattern::iterator, Data*>> CurrRpIters; 
  auto compare = [] (tuple<RepetitionPattern::iterator, RepetitionPattern::iterator, Data*>& a, tuple<RepetitionPattern::iterator, RepetitionPattern::iterator, Data*>& b) { 
    return get<0>(a)->first < get<0>(b)->first;
  };

  /* Get an iterator to the first entry in each RepetitionPattern */
  CurrRpIters rpIters;
  for(auto i = d.begin(); i != d.end(); i++){
    if(i->first.numRanges() > 0)
      rpIters.push_back(tuple<RepetitionPattern::iterator, RepetitionPattern::iterator, Data*>(i->first.begin(), i->first.end(), i->second));
  }

  /* Find the minimum entry, add it to the lut, advance the iterator and continue */
  while(rpIters.size() > 0){
    auto minRpIter = min_element(rpIters.begin(), rpIters.end(), compare);
    lut.push_back(tuple<uint64_t, uint64_t, Data*>(get<0>(*minRpIter)->first, get<0>(*minRpIter)->second, get<2>(*minRpIter)));
    get<0>(*minRpIter)++;
    if(get<0>(*minRpIter) == get<1>(*minRpIter)){
      rpIters.erase(minRpIter);
    }
  }
#endif

  /* Add all ranges to a map (automatically ordered) and then iterate the results into a vector */
  map<pair<uint64_t, uint64_t>, Data*> rangeMap;
  vector<tuple<uint64_t, uint64_t, Data*>> tempLut;
  for(auto i = d.begin(); i != d.end(); i++){
    for(auto j = i->first.begin(); j != i->first.end(); j++)
      rangeMap[pair<uint64_t, uint64_t>(j->first, j->second)] = i->second;
  }
  for(auto i = rangeMap.begin(); i != rangeMap.end(); i++)
    lut.push_back(tuple<uint64_t, uint64_t, Data*>(i->first.first, i->first.second, i->second));

  //for(auto i = tempLut.begin(), j = lut.begin(); i!= tempLut.end(); i++, j++)
  //  if(*i != *j) exit(-1);
}

template<class Data>
void RepetitionPatternLookup<Data>::clearLut(){
  lut.clear();
}

template<class Data>
uint64_t RepetitionPatternLookup<Data>::getLastInstance(){
  return get<1>(lut.back());
}

template<class Data>
Data* RepetitionPatternLookup<Data>::lookupData(uint64_t n){
  auto compare = [] (const tuple<uint64_t, uint64_t, Data*>& entry, uint64_t val){
    return get<1>(entry) < val;
  };
  if(lower_bound(lut.begin(), lut.end(), n, compare) == lut.end()){
    cout << "looked up " << n << " in \n" << *this << endl;
  }
  return get<2>(*lower_bound(lut.begin(), lut.end(), n, compare));
}

template<class Data>
typename RepetitionPatternLookup<Data>::iterator RepetitionPatternLookup<Data>::begin(){
  if(lut.begin() == lut.end())
    return end();
  else
    return iterator(lut.begin(), get<0>(*lut.begin()));
}

template<class Data>
typename RepetitionPatternLookup<Data>::iterator RepetitionPatternLookup<Data>::nextEntry(RepetitionPatternLookup<Data>::iterator iter){
  iter.first++;
  if(iter.first == lut.end())
    return end();
  else
    return iterator(iter.first, get<0>(*iter.first));
}

template<class Data>
typename RepetitionPatternLookup<Data>::iterator RepetitionPatternLookup<Data>::next(RepetitionPatternLookup<Data>::iterator iter){
  if(iter.second == get<1>(*iter.first)){ /* Reached the end of this entry */
    iter.first++;
    if(iter.first == lut.end())
      return end();
    else
      return iterator(iter.first, get<0>(*iter.first));
  }
  else{
    return iterator(iter.first, iter.second+1);
  }
}

template<class Data>
typename RepetitionPatternLookup<Data>::iterator RepetitionPatternLookup<Data>::end(){
  return iterator(lut.end(), 0);
}

template<class Data>
Data* RepetitionPatternLookup<Data>::getDataFromII(iterator iter){
  return get<2>(*iter.first);
}

template<class Data>
uint64_t RepetitionPatternLookup<Data>::getStartFromII(iterator iter){
  return get<0>(*iter.first);
}

template<class Data>
uint64_t RepetitionPatternLookup<Data>::getEndFromII(iterator iter){
  return get<1>(*iter.first);
}

template<class Data>
uint64_t RepetitionPatternLookup<Data>::getRangeNumFromII(iterator iter){
  return iter.second;
}
  
template<class Data>
ostream& operator<<(ostream& os, const RepetitionPatternLookup<Data>& rp){
  for(auto i = rp.lut.begin(); i != rp.lut.end(); i++){
    cout << get<0>(*i) << "-" << get<1>(*i) << ": " << get<2>(*i) << endl;
  }
  return os;
}


/*
 * RepetitionPatternLookup
*/

template<class Data>
typename RepetitionPatternSortedLookup<Data>::iterator RepetitionPatternSortedLookup<Data>::end(){
  return lut.end();
}

template<class Data>
void RepetitionPatternSortedLookup<Data>::insertToLut(const RepetitionPattern& rp, Data d){
  for(auto rpIter = rp.begin(); rpIter != rp.end(); rpIter++){
    lut.insert(tuple<uint64_t, uint64_t, Data>(rpIter->first, rpIter->second, d));
  }
}

template<class Data>
void RepetitionPatternSortedLookup<Data>::clearLut(){
  lut.clear();
}

template<class Data>
typename RepetitionPatternSortedLookup<Data>::iterator RepetitionPatternSortedLookup<Data>::lookupData(uint64_t n){
  auto it = lut.lower_bound(tuple<uint64_t, uint64_t, Data>(n, n, Data()));
  if(it == lut.end() || n < get<0>(*it))
    return lut.end();
  else 
    return it;
}

template<class Data>
Data RepetitionPatternSortedLookup<Data>::getDataFromIterator(RepetitionPatternSortedLookup<Data>::iterator i){
  return get<2>(*i);
}
  
template<class Data>
ostream& operator<<(ostream& os, const RepetitionPatternSortedLookup<Data>& rp){
  for(auto i = rp.lut.begin(); i != rp.lut.end(); i++){
    cout << get<0>(*i) << "-" << get<1>(*i) << ": " << endl;// << get<2>(*i) << endl;
  }
  return os;
}

/* 
 * Explicit instantiations of templates
*/

template class RepetitionPatternLookup<int>;
template ostream& operator<< <int>(ostream& os, const RepetitionPatternLookup<int>& rp);

template class RepetitionPatternLookup<CallTraceLoopInvocationGroup>;
template ostream& operator<< <CallTraceLoopInvocationGroup>(ostream& os, const RepetitionPatternLookup<CallTraceLoopInvocationGroup>& rp);

template class RepetitionPatternLookup<CallTraceInstanceGroup>;
template ostream& operator<< <CallTraceInstanceGroup>(ostream& os, const RepetitionPatternLookup<CallTraceInstanceGroup>& rp);

template class RepetitionPatternLookup<IterationGroupCfc>;
template ostream& operator<< <IterationGroupCfc>(ostream& os, const RepetitionPatternLookup<IterationGroupCfc>& rp);

template class RepetitionPatternSortedLookup<int>;
template ostream& operator<< <int>(ostream& os, const RepetitionPatternSortedLookup<int>& rp);

template class RepetitionPatternSortedLookup<StreamParseLoopRec::iterator>;
template ostream& operator<< <StreamParseLoopRec::iterator>(ostream& os, const RepetitionPatternSortedLookup<StreamParseLoopRec::iterator>& rp);
