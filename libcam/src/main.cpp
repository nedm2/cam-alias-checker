
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

#include <iostream>
#include <unistd.h>
#include <fstream>
#include <algorithm>
#include <map>
#include <string>
#include <time.h>
#include <stdlib.h>


#include "cam.h"
#include "loop_trace.hh"
#include "dependence_analysis.h"
#include "parser_wrappers.h"
#include <xanlib.h>
#include "RepetitionPattern.h"
#include "unit_tests.h"

using namespace std;


map<string, unsigned int> argsParser(int argc, char **argv){
  int c;
  map<string, unsigned int> args;
  args["bruteforce"] = 0;
  args["adjacent"] = 0;
  args["verbosity"] = 0;
  args["statistics"] = 0;
  args["deppairs"] = 0;
  args["checkstaticdepsonly"] = 0;
  args["check"] = 0;
  args["checkarg"] = 0;
  args["unit_tests"] = 0;
  while ((c = getopt (argc, argv, "abv:spqx:y:u:")) != -1){
    switch (c) {
      case 'a':
        args["adjacent"] = 1;
        break;
      case 'b':
        args["bruteforce"] = 1;
        break;
      case 'x':
        args["check"] = atoi(optarg);
        break;
      case 'y':
        args["checkarg"] = atoi(optarg);
        break;
      case 'v':
        args["verbosity"] = atoi(optarg);
        break;
      case 's':
        args["statistics"] = 1;
        break;
      case 'p':
        args["deppairs"] = 1;
        break;
      case 'q':
        args["checkstaticdepsonly"] = 1;
        break;
      case 'u':
        args["unit_tests"] = atoi(optarg);
        break;
      default:
        cerr << "Bad argument list\n";
        abort ();
    }
  }
  if(args["check"] || args["checkarg"]){
    if(!(args["check"] && args["checkarg"])){
      cout << "Use -x and -y together\n";
      exit(1);
    }
  }
  return args;
}

void my_handler(int s){
  printf("Caught signal %d\n",s);
  exit(1); 
}

void registerSigIntHandler(){
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);
}

/* Return (a0, a1) == (b0, b1) even if the order is reversed */
bool pairMatch(uintptr_t a0, uintptr_t a1, uintptr_t b0, uintptr_t b1){
  return ((a0 == b0 && a1 == b1) || (a0 == b1 && a1 == b0));
}

uint64_t checkDynamicDDGIsSubset(set<pair<uintptr_t, uintptr_t>> &pairs, vector<tuple<uintptr_t, uintptr_t, unsigned int>> &localStaticDDG, string& comments){
  string additional = "";
  uint64_t extraDeps = 0;
  for(auto dep = pairs.begin(); dep != pairs.end(); dep++){
    if(find_if(localStaticDDG.begin(), localStaticDDG.end(), 
             [&](const tuple<uintptr_t, uintptr_t, unsigned int>& d){
               return pairMatch(dep->first, dep->second, get<0>(d), get<1>(d));
             }
           ) == localStaticDDG.end()){
      extraDeps++;
      additional = "DDDG>SDDG(" + to_string(extraDeps) + ");";
      cout << "Dynamic dependence not found in static DDG: " << dep->first << " " << dep->second << endl;
    }
  }
  comments += additional;
  return extraDeps;
}

void doInstrumentationConsistencyCheck(
      pair<vector<uintptr_t>, vector<uintptr_t>> memtraceInstrIDs
    , set<uintptr_t> instrList
    , set<uintptr_t> callList
    , set<uintptr_t> subInstrList
    , string &comments){
  vector<uintptr_t> memtraceIDs;
  copy(memtraceInstrIDs.first.begin(), memtraceInstrIDs.first.end(), back_inserter(memtraceIDs));
  copy(memtraceInstrIDs.second.begin(), memtraceInstrIDs.second.end(), back_inserter(memtraceIDs));
  string additional = "";
  for(auto instr = memtraceIDs.begin(); instr != memtraceIDs.end(); instr++){
    if((instrList.find(*instr) == instrList.end()) && (subInstrList.find(*instr) == subInstrList.end())){
      additional = "memtrace > loop/calltrace;";
      cout << "Memory trace contained instruction not in call/loop trace: " << *instr << endl;
    }
  }
  comments += additional;
  additional = "";
  for(auto callid = callList.begin(); callid != callList.end(); callid++){
    if(instrList.find(*callid) == instrList.end()){
      additional = "calltrace > looptrace;";
      cout << "Call trace contained call ID not in loop trace: " << *callid << endl;
    }
  }
  comments += additional;
  additional = "";
  for(auto subInstr = subInstrList.begin(); subInstr != subInstrList.end(); subInstr++){
    if(find(memtraceIDs.begin(), memtraceIDs.end(), *subInstr) == memtraceIDs.end()){
      additional = "calltrace > memtrace;";
      cout << "Call trace contained sub instruction not in memory trace: " << *subInstr << endl;
    }
  }
  comments += additional;
}

bool setContainsPair(set<pair<uintptr_t, uintptr_t>> s, uintptr_t idA, uintptr_t idB){
  for(auto p : s){
    if(pairMatch(p.first, p.second, idA, idB))
      return true;
  }
  return false;
}

/* Takes the static dependence mask and filters out dependences not found dynamically */
unsigned int filterMask(set<pair<uintptr_t, uintptr_t>> reads, set<pair<uintptr_t, uintptr_t>> writes, 
                        uintptr_t idA, uintptr_t idB, unsigned int staticmask){
  if(staticmask & (DEP_MRAW | DEP_MWAR | DEP_RAW | DEP_WAR)){
    if(!setContainsPair(reads, idA, idB))
      //No WAR or RAW dependence detected, filter out all RAW/WAR bits
      staticmask = staticmask & (~(DEP_MRAW | DEP_MWAR | DEP_RAW | DEP_WAR));
  }
  if(staticmask & (DEP_MWAW | DEP_WAW)){
    if(!setContainsPair(writes, idA, idB))
      //No WAW dependence detected, filter out all WAW bits
      staticmask = staticmask & (~(DEP_MWAW | DEP_WAW));
  }
  return staticmask;
}

void produceStatistics(set<uintptr_t> instrList, set<uintptr_t> callList, set<uintptr_t> subInstrList){
  ofstream outputFile;
  auto pairs = parse_dependence_pairs();
  auto staticDDG = parse_static_ddg();

  uint64_t numRead = 0;
  uint64_t numWrite = 0;
  uint64_t numReadWrite = 0;
  uint64_t numCall = 0;
  uint64_t numStatic = 0;
  uint64_t numDynamic = 0;
  string comments = "";

  pair<vector<uintptr_t>, vector<uintptr_t>> memtraceInstrIDs = findMemoryTraces();

  doInstrumentationConsistencyCheck(memtraceInstrIDs, instrList, callList, subInstrList, comments);

  for(auto instr = memtraceInstrIDs.first.begin(); instr != memtraceInstrIDs.first.end(); instr++){
    if(find(memtraceInstrIDs.second.begin(), memtraceInstrIDs.second.end(), *instr) == memtraceInstrIDs.second.end())
      numRead++;
    else
      numReadWrite++;
  }
  for(auto instr = memtraceInstrIDs.second.begin(); instr != memtraceInstrIDs.second.end(); instr++){
    if(find(memtraceInstrIDs.first.begin(), memtraceInstrIDs.first.end(), *instr) == memtraceInstrIDs.first.end())
      numWrite++;
  }

  /* Find static DDG of this particular loop */
  vector<tuple<uintptr_t, uintptr_t, unsigned int>> localStaticDDG;
  for(auto dep = staticDDG.begin(); dep != staticDDG.end(); dep++){
    if(instrList.find(get<0>(*dep)) != instrList.end() &&
       instrList.find(get<1>(*dep)) != instrList.end()){
      localStaticDDG.push_back(*dep);
    }
  }
  numStatic = localStaticDDG.size();
  outputFile.open("static_ddg.txt");
  for(auto dep = localStaticDDG.begin(); dep != localStaticDDG.end(); dep++)
    outputFile << get<0>(*dep) << " " << get<1>(*dep) << " " << get<2>(*dep) << endl;
  outputFile.close();

  /* Find the number of dynamic dependences which match static dependences */
  set<tuple<uintptr_t, uintptr_t, unsigned int>> localDynamicDDG;
  for(auto dep = localStaticDDG.begin(); dep != localStaticDDG.end(); dep++){
    if(get<2>(*dep) & (DEP_MRAW | DEP_MWAR | DEP_RAW | DEP_WAR)){
      for(auto dyn = pairs.first.begin(); dyn != pairs.first.end(); dyn++){
        if (pairMatch(get<0>(*dep), get<1>(*dep), dyn->first, dyn->second)){
          localDynamicDDG.insert(*dep);
          break;
        }
      }
    }
    if(get<2>(*dep) & (DEP_MWAW | DEP_WAW)){
      for(auto dyn = pairs.second.begin(); dyn != pairs.second.end(); dyn++){
        if (pairMatch(get<0>(*dep), get<1>(*dep), dyn->first, dyn->second)){
          localDynamicDDG.insert(*dep);
          break;
        }
      }
    }
  }
  numDynamic = localDynamicDDG.size();
  outputFile.open("dynamic_ddg_static_subset.txt");
  for(auto dep = localDynamicDDG.begin(); dep != localDynamicDDG.end(); dep++)
    outputFile << get<0>(*dep) << " " << get<1>(*dep) << " " << get<2>(*dep) << endl;
  outputFile.close();

  /* Output the dynamic DDG, including all variable dependences and dependences outside the loop */
  vector<tuple<uintptr_t, uintptr_t, unsigned int>> dynamicDDG;
  for(auto staticdep : staticDDG){
    if(instrList.find(get<0>(staticdep)) != instrList.end() &&
       instrList.find(get<1>(staticdep)) != instrList.end()){
      /* Both instructions are inside the loop */
      unsigned int mask = filterMask(pairs.first, pairs.second, get<0>(staticdep), get<1>(staticdep), get<2>(staticdep));
      if(mask != 0)
        dynamicDDG.push_back(tuple<uintptr_t, uintptr_t, unsigned int>(get<0>(staticdep), get<1>(staticdep), mask));
    }
    else{
      /* If either instruction is outside the loop, the dependence must be included */
      dynamicDDG.push_back(staticdep);
    }
  }
  outputFile.open("dynamic_ddg.txt");
  for(auto i : dynamicDDG)
    outputFile << get<0>(i) << " " << get<1>(i) << " " << get<2>(i) << endl;
  outputFile.close();

  /* Output dependence pairs in ildjit format ( ID ID mask ) */
  //map<pair<uintptr_t, uintptr_t>, unsigned int> dynamicDDG;
  //for(auto i = pairs.first.begin(); i != pairs.first.end(); i++)
  //  dynamicDDG[pair<uintptr_t, uintptr_t>(i->first, i->second)] |= (DEP_MRAW | DEP_MWAR);
  //for(auto i = pairs.second.begin(); i != pairs.second.end(); i++)
  //  dynamicDDG[pair<uintptr_t, uintptr_t>(i->first, i->second)] |= (DEP_WAW);
  //outputFile.open("dynamic_ddg.txt");
  //for(auto i = dynamicDDG.begin(); i != dynamicDDG.end(); i++)
  //  outputFile << i->first.first << " " << i->first.second << " " << i->second << endl;
  //outputFile.close();

  /* Ensure all dynamic dependence pairs exist in static DDG */
  numDynamic += checkDynamicDDGIsSubset(pairs.first, localStaticDDG, comments);
  numDynamic += checkDynamicDDGIsSubset(pairs.second, localStaticDDG, comments);

  outputFile.open("cam_stats.csv");
  outputFile << "num_read_instr,num_write_instr,num_read_write_instr,num_call_instr,num_static_deps,num_dynamic_deps,comments\n";
  outputFile << numRead << ","
             << numWrite << ","
             << numReadWrite << ","
             << numCall << ","
             << numStatic << ","
             << numDynamic << ","
             << comments << endl
             << endl;
  outputFile.close();
}

int main(int argc, char **argv){
  
  registerSigIntHandler();

  map<string, unsigned int> args = argsParser(argc, argv);

  if(args["unit_tests"])
    run_unit_tests(args["unit_tests"]);

  StreamParseCallTrace callTraces;
  pair<set<uintptr_t>, uint64_t> instrListAndNumInvoc; 
  pair<set<uintptr_t>, set<uintptr_t>> callTraceList;
  if(args["deppairs"] == 1 || args["statistics"] == 1){
    instrListAndNumInvoc = parseLoopTraceForInstrList();
    callTraceList = parseCallTraceForInstrList();
    if(args["deppairs"] == 1){
      PDEBUG("Parse call trace\n");
      callTraces = parseCallTrace();
    }
  }

  if(args["deppairs"] == 1)
    dependence_analysis(args, callTraces, instrListAndNumInvoc.first, instrListAndNumInvoc.second, callTraceList.first, callTraceList.second);

  if(args["statistics"] == 1)
    produceStatistics(instrListAndNumInvoc.first, callTraceList.first, callTraceList.second);

  bool optionSet = false;
  for(auto opt : args)
    if(opt.second)
      optionSet = true;
  if(!optionSet){
    #if __x86_64__ || __ppc64__
      cout << "cam: 64 bit\n";
    #else
      cout << "cam: 32 bit\n";
    #endif
  }
}
