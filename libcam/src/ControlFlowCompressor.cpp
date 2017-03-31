
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

#include "ControlFlowCompressor.h"

ostream& operator<<(ostream& os, const TracerCompressionPattern& p){
  if(p.pattern.size() > 0){
    os << "(";
    for (auto pat = p.pattern.begin(); pat != p.pattern.end(); pat++){
      os << **pat;
    }
    os << "," << p.numRepetitions << ")";
  }
  else{
    os << "(" << p.sym << ")";
  }
  return os;
}

void TracerCompressionPattern::coutThis(){
  cout << *this;
}

bool TracerCompressionPattern::operator==(const TracerCompressionPattern& other) const {
  if(pattern.size() != other.pattern.size())
    return false;
  else if(pattern.size() == 0 && other.pattern.size() == 0)
    return sym == other.sym;
  else if(numRepetitions != other.numRepetitions)
    return false;
  else{
    auto i = pattern.begin();
    auto j = other.pattern.begin();
    for(; i != pattern.end(); i++, j++){
      if(!(**i == **j))
        return false;
    }
    return true;
  }
}

bool TracerCompressionPattern::operator!=(const TracerCompressionPattern& other) const {
  return !(*this == other);
}

/* Important!!: This constructor steals ownership of the passed patterns, do not delete in caller */
TracerCompressionPattern::TracerCompressionPattern(CamMemoryAllocator* alloc, vector<TracerCompressionPattern*> pattern_p, uint64_t numRepetitions_p)
  : allocator(alloc), numRepetitions(numRepetitions_p) {
  for(auto cp = pattern_p.begin(); cp != pattern_p.end(); cp++){
    pattern.push_back(*cp);
  }
}

TracerCompressionPattern::TracerCompressionPattern(CamMemoryAllocator* alloc, const TracerCompressionPattern& other)
  : allocator(alloc) {
  if(other.pattern.size() > 0){
    numRepetitions = other.numRepetitions;
    for(auto cp = other.pattern.begin(); cp != other.pattern.end(); cp++){
      pattern.push_back(allocator->newMem<TracerCompressionPattern>(**cp));
    }
  }
  else{
    sym = other.sym;
  }
}

TracerCompressionPattern::TracerCompressionPattern(CamMemoryAllocator* alloc, tracer_symbol sym_p)
  : allocator(alloc), sym(sym_p) {}

/* Only to be used when the pattern is being built on the fly */
TracerCompressionPattern::TracerCompressionPattern(CamMemoryAllocator* alloc)
  : allocator(alloc) {
}

TracerCompressionPattern& TracerCompressionPattern::operator=(const TracerCompressionPattern& other){
  if(this != &other){
    if(other.pattern.size() > 0){
      // Acquire
      vector<TracerCompressionPattern*> v;
      for(auto cp = other.pattern.begin(); cp != other.pattern.end(); cp++){
        v.push_back(allocator->newMem<TracerCompressionPattern>(**cp));
      }
      //Release
      for(auto cp = this->pattern.begin(); cp != this->pattern.end(); cp++){
        allocator->deleteMem<TracerCompressionPattern>(*cp);
      }
      this->pattern.clear();
      //Assign
      this->pattern = v;
      this->numRepetitions = other.numRepetitions;
    }
    else{
      this->sym = other.sym;
      this->pattern.clear();
    }
  }
  return *this;
}

TracerCompressionPattern::~TracerCompressionPattern(){
  for(auto cp = pattern.begin(); cp != pattern.end(); cp++){
    allocator->deleteMem<TracerCompressionPattern>(*cp);
  }
  pattern.clear();
}

void TracerCompressionPattern::incRepetitions(){
  numRepetitions++;
}

bool TracerCompressionPattern::patternMatchesSymbols(deque<TracerCompressionPattern*> &symbols){
  auto sym = symbols.begin();
  auto pat = pattern.end();
  for(; pat != pattern.begin(); sym++){
    pat--;
    if(**sym != **pat)
      return false;
  }
  return true;
}

unsigned int TracerCompressionPattern::getPatternSize(){
  return pattern.size();
}

vector<tracer_symbol> TracerCompressionPattern::decompress(){
  vector<tracer_symbol> v;
  if(pattern.size() > 0){
    for(unsigned int i = 0; i < numRepetitions; i++){
      for(auto cp = pattern.begin(); cp != pattern.end(); cp++){
        vector<tracer_symbol> cpSyms = (**cp).decompress();
        v.insert(v.end(), cpSyms.begin(), cpSyms.end());
      }
    }
  }
  else
    v.push_back(sym);
  return v;
}

void TracerCompressionPattern::addPattern(TracerCompressionPattern& cp){
  pattern.push_back(allocator->newMem<TracerCompressionPattern>(cp));
}

void TracerCompressionPattern::setNumRepetitions(uint64_t n){
  numRepetitions = n;
}

void TracerCompressionPattern::setSymbol(tracer_symbol s){
  sym = s;
}

map<tracer_symbol, uint64_t> TracerCompressionPattern::getNumberOfInstancesMap(){
  map<tracer_symbol, uint64_t> numInstances;
  if(pattern.empty()){
    numInstances[sym] = 1;
  }
  else{
    for(auto cp = pattern.begin(); cp != pattern.end(); cp++){
      map<tracer_symbol, uint64_t> innerNumInstances = (**cp).getNumberOfInstancesMap();
      for(auto inner = innerNumInstances.begin(); inner != innerNumInstances.end(); inner++){
        numInstances[inner->first] += numRepetitions*inner->second;
      }
    }
  }
  return numInstances;
}


/*
 * ControlFlowCompressor
*/

ControlFlowCompressor::ControlFlowCompressor(CamMemoryAllocator* alloc) 
  : allocator(alloc), maxWindowLength(100) { }

ControlFlowCompressor::ControlFlowCompressor(CamMemoryAllocator* alloc, unsigned int maxWindowLength_p) 
  : allocator(alloc), maxWindowLength(maxWindowLength_p) { }

ControlFlowCompressor::~ControlFlowCompressor(){
  for(auto i = window.begin(); i != window.end(); i++)
    allocator->deleteMem<TracerCompressionPattern>(*i);
  for(auto i = storedPatterns.begin(); i != storedPatterns.end(); i++)
    allocator->deleteMem<TracerCompressionPattern>(*i);
}

vector<tracer_symbol> ControlFlowCompressor::decompress(){
  vector<tracer_symbol> v;
  for(auto cp = storedPatterns.begin(); cp != storedPatterns.end(); cp++){
    vector<tracer_symbol> cpSyms = (**cp).decompress();
    v.insert(v.end(), cpSyms.begin(), cpSyms.end());
  }
  for(auto cp = window.end(); cp != window.begin();){
    cp--;
    vector<tracer_symbol> cpSyms = (**cp).decompress();
    v.insert(v.end(), cpSyms.begin(), cpSyms.end());
  }
  return v;
}

bool ControlFlowCompressor::isRepeatedSequence(unsigned int width){
  for(unsigned int index = 0; index < width; index++){
    if(*window[index] != *window[index + width])
      return false;
  }
  return true;
}

bool ControlFlowCompressor::match(){
  if(window.empty())
    return false;
  /* Starting from the oldest pattern and see if it matches the entire sequenece of younger symbols */
  for(unsigned int patternIndex = min((maxWindowLength/2)+1, (unsigned int)(window.size() - 1)); patternIndex >= 1; patternIndex--){
    if(patternIndex == window[patternIndex]->getPatternSize() && window[patternIndex]->patternMatchesSymbols(window)){
      for(unsigned int i = 0; i < patternIndex; i++){
        allocator->deleteMem<TracerCompressionPattern>(window.front());
        window.pop_front();
      }
      window[0]->incRepetitions();
      return true;  
    }
  }
  return false;
}

bool ControlFlowCompressor::merge(){
  for(unsigned int width = window.size()/2; width > 0; width--){

    /* Check if two consecutive sequences are identical */
    if(isRepeatedSequence(width)){
      vector<TracerCompressionPattern*> v;

      /* Create vector of patterns (younger half of patterns) as input for new pattern */
      for(unsigned int i = width; i > 0;){
        i--;
        v.push_back(window[i]);
      }

      /* Remove them from the window and delete the duplicate (older half of patterns) */
      for(unsigned int i = 0; i < width*2; i++){
        if(i >= width)
          allocator->deleteMem<TracerCompressionPattern>(window.front());
        window.pop_front();
      }

      /* Create a new TracerCompressionPattern from the younger half of patterns */
      window.push_front(allocator->newMem<TracerCompressionPattern>(allocator, v, 2));
      return true;
    }
  }
  return false;
}

void ControlFlowCompressor::insertSymbol(tracer_symbol symbol){

  /* Push new symbol into the window */
  window.push_front(allocator->newMem<TracerCompressionPattern>(allocator, symbol));

  /* Match and merge until the window does not change */
  while( match() || merge() );

  /* If the window is overflowing, pop the oldest entry into permanent storage */
  if(window.size() > maxWindowLength){
    storedPatterns.push_back(window.back());
    window.pop_back();
  }
}

ControlFlowCompressor::CFCIterator ControlFlowCompressor::iteratorBegin() const {
  if(storedPatterns.size() > 0)
    return CFCIterator(storedPatterns.begin(), window.rbegin(), 0);
  else
    return CFCIterator(storedPatterns.begin(), window.rbegin(), 1);
}

ControlFlowCompressor::CFCIterator ControlFlowCompressor::iteratorNext(CFCIterator iter) const {
  if (get<2>(iter) == 0 && next(get<0>(iter), 1) == storedPatterns.end())
    return CFCIterator(storedPatterns.end(), window.rbegin(), 1);
  else if(get<2>(iter) == 0)
    return CFCIterator(next(get<0>(iter), 1), window.rbegin(), 0);
  else
    return CFCIterator(storedPatterns.end(), next(get<1>(iter), 1), 1);
}

ControlFlowCompressor::CFCIterator ControlFlowCompressor::iteratorEnd() const {
  return CFCIterator(storedPatterns.end(), window.rend(), 1);
}

const TracerCompressionPattern *ControlFlowCompressor::iteratorData(CFCIterator iter) const {
  if(get<2>(iter) == 0)
    return *(get<0>(iter));
  else
    return *(get<1>(iter));
}

void ControlFlowCompressor::clear(){
  for(auto i = window.begin(); i != window.end(); i++)
    allocator->deleteMem<TracerCompressionPattern>(*i);
  for(auto i = storedPatterns.begin(); i != storedPatterns.end(); i++)
    allocator->deleteMem<TracerCompressionPattern>(*i);
  storedPatterns.clear();
  window.clear();
}

bool ControlFlowCompressor::isEmpty(){
  return (storedPatterns.size() == 0 && window.size() == 0);
}

bool ControlFlowCompressor::operator==(const ControlFlowCompressor& other){
  if(storedPatterns.size() + window.size() != other.storedPatterns.size() + other.window.size())
    return false;
  else{
    ControlFlowCompressor::CFCIterator thisIter = iteratorBegin();
    ControlFlowCompressor::CFCIterator otherIter = other.iteratorBegin();
    for(;thisIter != iteratorEnd(); thisIter = iteratorNext(thisIter), otherIter = other.iteratorNext(otherIter))
      if(*(iteratorData(thisIter)) != *(other.iteratorData(otherIter)))
        return false;
  }
  return true;
}

bool ControlFlowCompressor::operator!=(const ControlFlowCompressor& other){
  return !(*this == other);
}

void ControlFlowCompressor::coutThis(){
  cout << *this;
}


ostream& operator<<(ostream& os, const ControlFlowCompressor& cfc){
  for(auto i = cfc.storedPatterns.begin(); i != cfc.storedPatterns.end(); i++)
    os << **i;
  for(auto i = cfc.window.rbegin(); i != cfc.window.rend(); i++)
    os << **i;
  return os;
}

void ControlFlowCompressor::rawOutput(){
  cout << "WINDOW: \n";
  for(auto i = window.begin(); i != window.end(); i++)
    cout << **i << endl;
  cout << "STORED: \n";
  for(auto i = storedPatterns.begin(); i != storedPatterns.end(); i++)
    cout << **i << endl;
}

