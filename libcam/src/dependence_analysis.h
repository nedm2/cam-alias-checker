
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

#ifndef DEPENDENCE_ANALYSIS_H
#define DEPENDENCE_ANALYSIS_H

#include <set>
#include "static_inst_rec.h"
#include "StaticLoopRec.h"
#include "dynamic_gcd.h"
#include "CallTrace.h"

typedef tuple<uintptr_t, MemSetEntry&> AccessRange;
typedef tuple<uintptr_t, MemSetEntry&, vector<AccessRange>> AccessMultiMap;

class AliasRec{
  uintptr_t instrA;
  uintptr_t instrB;
  MemSetEntry &setA;
  MemSetEntry &setB;

public: 
  uintptr_t getInstrA() const;
  uintptr_t getInstrB() const;
  MemSetEntry &getSetA() const;
  MemSetEntry &getSetB() const;
  AliasRec(uintptr_t instrA_par, MemSetEntry &a_par, uintptr_t instrB_par, MemSetEntry &b_par)
    : instrA(instrA_par), instrB(instrB_par), setA(a_par), setB(b_par) { }
  virtual ostream& print(ostream &os) const = 0;
  //virtual bool allAliasesAreIntraIteration(InvocationGroup &invGroup, uint64_t invocNum, CallTraces& callTraces) = 0;
  //virtual set<pair<uintptr_t, uintptr_t>> getSetOfCallPairs(CallTraces& invCallTraces) = 0;
  friend ostream& operator<<(ostream& os, const AliasRec& a);
};

class BruteForceAlias : public AliasRec{
  vector<tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>> deps; // addressA, memSetInstanceA, addressB, memSetInstanceB

public:
  BruteForceAlias(uintptr_t instrA_par, MemSetEntry &a_par, uintptr_t instrB_par, MemSetEntry &b_par, vector<tuple<uintptr_t, uint64_t, uintptr_t, uint64_t>> deps_par) 
    : AliasRec(instrA_par, a_par, instrB_par, b_par), deps(deps_par) { }
  virtual ostream& print(ostream& os) const { return os << *this; }
  //virtual bool allAliasesAreIntraIteration(InvocationGroup &invGroup, uint64_t invocNum, CallTraces& callTraces);
  //set<pair<uintptr_t, uintptr_t>> getSetOfCallPairs(CallTraces& invCallTraces);
  friend ostream& operator<<(ostream& os, const BruteForceAlias& a);
};

class PatternAlias : public AliasRec{
  AliasT alias;

public:
  AliasT getAlias() const;
  PatternAlias(uintptr_t instrA_par, MemSetEntry &a_par, uintptr_t instrB_par, MemSetEntry &b_par, AliasT alias_par) 
    : AliasRec(instrA_par, a_par, instrB_par, b_par), alias(alias_par) {}
  virtual ostream& print(ostream& os) const { return os << *this; }
  //bool allAliasesAreIntraIteration(InvocationGroup &invGroup, uint64_t invocNum, CallTraces& callTraces);
  //set<pair<uintptr_t, uintptr_t>> getSetOfCallPairs(CallTraces& invCallTraces);
  BruteForceAlias convertToBruteForceAlias();
  friend ostream& operator<<(ostream& os, const PatternAlias& a);
};

void dependence_analysis(map<string, unsigned int> &args, StreamParseCallTrace &callTraces, 
  set<uintptr_t> instrList, uint64_t numInvocations, set<uintptr_t> callList, set<uintptr_t> subInstrList);
//vector<uintptr_t> getInstructionIDs(MemoryTrace &memoryTrace, StaticLoopRec &loop, CallTraces &callTraces);


void outputDependencePairs(set<pair<uintptr_t, uintptr_t>> &rw_dependence_pairs, set<pair<uintptr_t, uintptr_t>> &ww_dependence_pairs);

#endif
