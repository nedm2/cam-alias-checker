
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

#include "CompressionPattern.h"
#include <cassert>

/* 
 * CompressionPatternChild
*/

ostream& operator<<(ostream& os, const CompressionPatternChild& p){
  os << "(";
  for(auto cp : p.pattern)
    os << cp;
  os << "," << p.numRepetitions << ")";
  return os;
}

CompressionPatternChild::CompressionPatternChild(){
}

bool CompressionPatternChild::operator==(const CompressionPatternChild& other) const {
  if(pattern.size() != other.pattern.size())
    return false;
  else if(numRepetitions != other.numRepetitions)
    return false;
  else{
    auto i = pattern.begin();
    auto j = other.pattern.begin();
    for(; i != pattern.end(); i++, j++){
      if(!(*i == *j))
        return false;
    }
    return true;
  }
}

void CompressionPatternChild::push_back(CompressionPattern cp){
  pattern.push_back(cp);
}

void CompressionPatternChild::setNumRepetitions(uint64_t n){
  numRepetitions = n;
}

void CompressionPatternChild::incRepetitions(){
  numRepetitions++;
}

unsigned int CompressionPatternChild::getPatternSize(){
  return pattern.size();
}

map<c_symbol, uint64_t> CompressionPatternChild::getNumberOfInstancesMap(){
  map<c_symbol, uint64_t> numInstances;
  for(auto cp : pattern){
    map<c_symbol, uint64_t> innerNumInstances = cp.getNumberOfInstancesMap();
    for(auto inner : innerNumInstances){
      numInstances[inner.first] += numRepetitions*inner.second;
    }
  }
  return numInstances;
}

/* 
 * CompressionPattern
*/

ostream& operator<<(ostream& os, const CompressionPattern& p){
  if(p.hasChild()){
    os << *p.child;
  }
  else{
    os << "(" << p.sym << ")";
  }
  return os;
}

bool CompressionPattern::operator==(const CompressionPattern& other) const {
  if(hasChild()){
    if(!other.hasChild()) return false;
    else return *child == *other.child;
  }
  else{
    return sym == other.sym;
  }
}

bool CompressionPattern::operator!=(const CompressionPattern& other) const {
  return !(*this == other);
}

#if 0

/* Important!!: This constructor steals ownership of the passed patterns, do not delete in caller */
CompressionPattern::CompressionPattern(vector<CompressionPattern*> pattern_p, uint64_t numRepetitions_p)
  : numRepetitions(numRepetitions_p) {
  for(auto cp = pattern_p.begin(); cp != pattern_p.end(); cp++){
    pattern.push_back(*cp);
  }
}
#endif

CompressionPattern::CompressionPattern(const CompressionPattern& other) : sym(0), child(NULL) {
  if(other.hasChild()){
    child = new CompressionPatternChild(*other.child);
  }
  else{
    sym = other.sym;
  }
}


/* Only to be used when the pattern is being built on the fly */
CompressionPattern::CompressionPattern() : sym(0), child(NULL) { }

CompressionPattern::CompressionPattern(c_symbol sym_p) : sym(sym_p), child(NULL)  { }

CompressionPattern& CompressionPattern::operator=(const CompressionPattern& other){
  if(this != &other){
    if(other.hasChild()){
      // Acquire
      CompressionPatternChild *c = new CompressionPatternChild(*other.child);
      //Release
      if(hasChild()){
        delete child;
        child = NULL;
      }
      //Assign
      this->child = c;
    }
    else{
      this->sym = other.sym;
      this->child = NULL;
    }
  }
  return *this;
}

CompressionPattern::~CompressionPattern(){
  if(hasChild())
    delete child;
}


void CompressionPattern::incRepetitions(){
  assert(hasChild());
  child->incRepetitions();
}


#if 0
bool CompressionPattern::patternMatchesSymbols(deque<CompressionPattern*> &symbols){
  auto sym = symbols.begin();
  auto pat = pattern.end();
  for(; pat != pattern.begin(); sym++){
    pat--;
    if(**sym != **pat)
      return false;
  }
  return true;
}
#endif

unsigned int CompressionPattern::getPatternSize(){
  assert(hasChild());
  return child->getPatternSize();
}

#if 0

vector<c_symbol> CompressionPattern::decompress(){
  vector<c_symbol> v;
  if(pattern.size() > 0){
    for(unsigned int i = 0; i < numRepetitions; i++){
      for(auto cp = pattern.begin(); cp != pattern.end(); cp++){
        vector<c_symbol> cpSyms = (**cp).decompress();
        v.insert(v.end(), cpSyms.begin(), cpSyms.end());
      }
    }
  }
  else
    v.push_back(sym);
  return v;
}
#endif


void CompressionPattern::addPattern(CompressionPattern& cp){
  if(!hasChild())
    child = new CompressionPatternChild();
  child->push_back(cp);
}


void CompressionPattern::setNumRepetitions(uint64_t n){
  assert(hasChild());
  child->setNumRepetitions(n);
}


void CompressionPattern::setSymbol(c_symbol s){
  sym = s;
}

bool CompressionPattern::hasChild() const {
  return child != NULL;
}

map<c_symbol, uint64_t> CompressionPattern::getNumberOfInstancesMap(){
  map<c_symbol, uint64_t> numInstances;
  if(!hasChild()){
    numInstances[sym] = 1;
  }
  else{
    return child->getNumberOfInstancesMap();
  }
  return numInstances;
}

