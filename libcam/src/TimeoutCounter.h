
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

#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class TimeoutCounter{
  int initTime;
  int timeout;
  bool timedOut;
  uint64_t numOperations;
  uint64_t numOperationsAtTimeout;
  string outputFileName;

public:
  TimeoutCounter() 
    : initTime(time(NULL)), timedOut(false), numOperations(0), numOperationsAtTimeout(0), outputFileName("timeout_stats.csv") {
    char *env = getenv("LIBCAM_TIMEOUT");
    if (env) 
      timeout = atoi(env);
    else 
      timeout = 10800; //default timeout: 3 hours
  }

  TimeoutCounter(int t) 
    : initTime(time(NULL)), timeout(t), timedOut(false), numOperations(0), numOperationsAtTimeout(0), outputFileName("timeout_stats.csv") {}

  bool isTimedOut(){ return timedOut; }

  void checkTime(){ 
    if(time(NULL) > initTime + timeout){
      timedOut = true;
      numOperationsAtTimeout = numOperations;
    }
  }

  void addOperation(){ numOperations++; }

  uint64_t getNumOperations() { return numOperations; }
  uint64_t getNumOperationsAtTimeout() { return numOperationsAtTimeout; }

  bool recordOperation() {
    addOperation();
    if(isTimedOut()){
      return true;
    }
    else{
      if(numOperations % 1000000 == 0)
        checkTime();
      return false;
    }
  }

  void dumpStats(string outputDirectory){
    ofstream outputFile;
    outputFile.open(outputDirectory + "/" + outputFileName);
    outputFile << "timeout,has_timed_out,operations_processed,total_operations\n";
    outputFile << timeout << ","
               << timedOut << ","
               << numOperationsAtTimeout << ","
               << numOperations 
               << endl << endl;
    outputFile.close();
    if(timedOut){
      cerr << "Trace timed out after " << timeout << " seconds. See timeout_stats.csv for details." << endl;
      cerr << "Timeout value can be set with LIBCAM_TIMEOUT." << endl;
    }
  }

  void dumpStats(string filename, uint64_t operations_processed, uint64_t total_operations){
    numOperationsAtTimeout = operations_processed;
    numOperations = total_operations;
    outputFileName = filename;
    dumpStats(".");
  }
};
