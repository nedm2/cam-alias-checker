
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

#include "unit_tests.h"
#include "cam.h"
#include "CallTrace.h"
#include "StaticLoopRec.h"
#include "static_inst_rec.h"
#include "IntervalTree.h"
#include "parser_wrappers.h"
#include "memory_allocator.hh"
#include "ControlFlowCompressor.h"
#include "dependence_analysis.h"
#include <random>
#include <time.h>
#include <assert.h>
#include <chrono>

typedef Interval<bool, uint64_t> interval;
typedef vector<interval> intervalVector;
typedef IntervalTree<bool, uint64_t> intervalTree;

template<typename K>
K randKey(K floor, K ceiling) {
    K range = ceiling - floor;
    return floor + range * ((double) rand() / (double) (RAND_MAX + 1.0));
}

template<class T, typename K>
Interval<T,K> randomInterval(K maxStart, K maxLength, K maxStop, const T& value) {
    K start = randKey<K>(0, maxStart);
    K stop = min<K>(randKey<K>(start, start + maxLength), maxStop);
    return Interval<T,K>(start, stop, value);
}  

int testIntervalTreeCorrectness() {
    typedef vector<std::size_t> countsVector;

    // a simple sanity check
    intervalVector sanityIntervals;
    sanityIntervals.push_back(interval(60, 80, true));
    sanityIntervals.push_back(interval(20, 40, true));
    intervalTree sanityTree(sanityIntervals);

    intervalVector sanityResults;
    sanityTree.findOverlapping(30, 50, sanityResults);
    assert(sanityResults.size() == 1);
    sanityResults.clear();
    sanityTree.findContained(15, 45, sanityResults);
    assert(sanityResults.size() == 1);
     

    srand((unsigned)time(NULL));

    intervalVector intervals;
    intervalVector queries;
    
    // generate a test set of target intervals
    for (uint64_t i = 0; i < 1000000; ++i) {
        intervals.push_back(randomInterval<bool, uint64_t>(100000, 1000, 100000 + 1, true));
    }
    // and queries
    for (uint64_t i = 0; i < 5000; ++i) {
        queries.push_back(randomInterval<bool, uint64_t>(100000, 1000, 100000 + 1, true));
    }

    typedef chrono::high_resolution_clock Clock;
    typedef chrono::milliseconds milliseconds;

    Clock::time_point t0;
    Clock::time_point t1;
    milliseconds ms;

#if 0
    // using brute-force search
    countsVector bruteforcecounts;
    t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        for (intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
            if (i->start <= q->stop && i->stop >= q->start) {
                results.push_back(*i);
            }
        }
        bruteforcecounts.push_back(results.size());
    }
    t1 = Clock::now();
    ms = chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "brute force:\t" << ms.count() << "ms" << endl;
#endif

    // using the interval tree
    intervalTree tree = intervalTree(intervals);
    countsVector treecounts;
    t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        tree.findOverlapping(q->start, q->stop, results);
        treecounts.push_back(results.size());
    }
    t1 = Clock::now();
    ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "interval tree:\t" << ms.count() << "ms" << endl;

    // using the interval tree
    intervalTree itertree = intervalTree(intervals);
    countsVector itertreecounts;
    t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        itertree.findOverlappingWithIterator(q->start, q->stop, results);
        itertreecounts.push_back(results.size());
    }
    t1 = Clock::now();
    ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "interval tree iterator:\t" << ms.count() << "ms" << endl;

    // check that the same number of results are returned
    countsVector::iterator it = itertreecounts.begin();
    for (countsVector::iterator t = treecounts.begin(); t != treecounts.end(); ++t, ++it) {
        assert(*it == *t);
    }

    return 0;
}

void testIntervalTree(){

  intervalVector intervals;
  intervalVector queries;

  // generate a test set of target intervals
  for (int i = 0; i < 100000000; ++i) {
      intervals.push_back(randomInterval<bool, uint64_t>(1000, 100, 1000 + 1, true));
  }
  // and queries
  for (int i = 0; i < 5000; ++i) {
      queries.push_back(randomInterval<bool, uint64_t>(1000, 100, 1000 + 1, true));
  }

  intervalTree itree = intervalTree(intervals);
  itree.printStats();
  return;

  for(auto q = queries.begin(); q != queries.end(); q++){
    intervalVector overlaps;
    itree.findOverlapping(q->start, q->stop, overlaps);
    auto it = overlaps.begin(); 
    for(intervalTree::OverlapIterator i = itree.overlapIteratorBegin(q->start, q->stop); i != itree.overlapIteratorEnd(); i = itree.overlapIteratorNext(q->start, q->stop, i), it++){
      if(i.second->start != it->start || i.second->stop != it->stop)
        cout << "error\n";
    }
    if(it != overlaps.end())
      cout << "Error\n";
  }
}

void testRepetitionPattern(){
  vector<RepetitionPattern> rps;
  for(int i = 0; i < 10; i++){
    rps.push_back(RepetitionPattern());
    for(int j = 0; j < 10; j++)
      rps.back().addPair((100*j) + (i*2), (100*j)+(i*2)+1);
  }
  for(auto i = rps.begin(); i != rps.end(); i++)
    cout << *i << endl;
  RepetitionPatternLookup<int> rpl;
  vector<pair<RepetitionPattern&, int *>> rplInput;
  int blerg[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int j = 0;
  for(auto i = rps.begin(); i != rps.end(); i++, j++)
    rplInput.push_back(pair<RepetitionPattern&, int *>(*i, &blerg[j]));
  rpl.buildLut(rplInput);
  cout << rpl << endl;
}

void testRepetitionPatternSortedLookup(){
  RepetitionPatternSortedLookup<int> lu;
  vector<int> ints;
  set<int> checker;
  vector<int> values;
  for(int i = 0; i < 10; i++){
    int val = randKey<int>(0, 20);
    auto res = checker.insert(val);
    if(res.second)
      values.push_back(val);
  }
  //for(auto i = values.begin(); i != values.end(); i++)
  //  cout << *i << endl;
  //cout << "size: " << values.size();
  for(auto i = values.begin(); i != values.end(); i++){
    RepetitionPattern rp;
    rp.addPair((*i)*1000, (*i)*1000 + 500);
    ints.push_back(*i);
    lu.insertToLut(rp, ints.back());
  }
  cout << lu << endl;
  auto it = lu.lookupData(1500);
  if(it == lu.end())
    cout << 1500 << ": " << "end" << endl;
  else
    cout << 1500 << ": " << lu.getDataFromIterator(it) << endl;

}

//void testCompressionPattern(){
//  int numSyms = 1000000;
//  vector<int> syms;
//  for(auto i = 0; i < numSyms; i++)
//    syms.push_back(i%20 + (rand()%10)/9);
//  for(auto i : syms)
//    CompressionPattern.
//}

void testLoopTraceStreamParseFirstPass(){
  pair<set<uintptr_t>, uint64_t> instrListEtc = parseLoopTraceForInstrList();
  cout << "Num invocations: " << instrListEtc.second << endl;
  for(auto i = instrListEtc.first.begin(); i != instrListEtc.first.end(); i++)
    cout << *i << " ";
  cout << endl;
}

void testMemoryTraceParse(){
  MemoryTrace t = parse_memory_trace();
  cout << t << endl;
}

void testDepPairsParse(){
  auto pairs = parse_dependence_pairs();
  outputDependencePairs(pairs.first, pairs.second);
}

void testStaticDDGParse(){
  auto ddg = parse_static_ddg();
  for (auto i = ddg.begin(); i != ddg.end(); i++){
    cout << get<0>(*i) << " " << get<1>(*i) << " " << get<2>(*i) << endl;
  }
}


void testCallTraceParse(){
  //CallTraceLoopInvocationGroup invCT;
  //int status = callTraceGetNextEntry(invCT);
  //while(status){
  //  cout << invCT << endl;
  //  invCT = CallTraceLoopInvocationGroup();
  //  status = callTraceGetNextEntry(invCT);
  //}
  StreamParseCallTrace ct = parseCallTrace();
  cout << ct << endl;
}

void testCallTraceParseFirstPass(){
  pair<set<uintptr_t>, set<uintptr_t>> lists = parseCallTraceForInstrList();
  cout << "calls: ";
  for (auto i = lists.first.begin(); i != lists.first.end(); i++)
    cout << *i << " ";
  cout << endl;
  cout << "instructions: ";
  for (auto i = lists.second.begin(); i != lists.second.end(); i++)
    cout << *i << " ";
  cout << endl;
}

void run_unit_tests(int test){
  srand (time(NULL));
  switch(test){
    case 1:
      testCallTraceParseFirstPass();
      break;
    default:
      cout << "Specify test\n";
      break;
  }
}
