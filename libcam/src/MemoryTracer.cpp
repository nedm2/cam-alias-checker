
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

#include <bzlib.h>
#include <stdint.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "cam.h"
#include "cam_system.h"
#include "MemoryTracer.h"
#include "memory_allocator.hh"
#include "TimeoutCounter.h"

#include <fstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

/**
 * Maximum amount of memory to use before dumping the trace.  Can be altered
 * using the environment variable MEM_TRACE_MAX_MEM_USAGE.
 **/
#define LIBCAM_DEFAULT_MAX_MEM_USAGE ((JITUINT64)1073741824ULL)

class TracerMemSetEntry {
  uintptr_t base;
  intptr_t stride;
  uint64_t length;
  uint64_t start;
  uint64_t end;
public:
  void init(uintptr_t b, intptr_t s, uint64_t l, uint64_t st, uint64_t e);

  /* Return the next address as predicted by the pattern */
  uintptr_t prediction();

  uintptr_t getBase() const;
  intptr_t getStride() const;
  uintptr_t getLength() const;
  uintptr_t getStart() const;
  uintptr_t getEnd() const;
  void setBase(uintptr_t);
  void setStride(intptr_t);
  void setLength(uint64_t);
  void setStart(uint64_t);
  void setEnd(uint64_t);

  uintptr_t getNumInstances();
  void incEnd();
  void dumpInitialEntry(BZFILE *compressedFile);
  void dumpEntry(BZFILE *compressedFile, TracerMemSetEntry* prev);
};


class TracerMemSet : public std::vector<TracerMemSetEntry *>{
public:
  ~TracerMemSet();
  void newMemSetEntry(uintptr_t b, intptr_t s, uint64_t l, uint64_t st, uint64_t e);
  void recordMemoryReference(uintptr_t addr, uint64_t len);
  void dumpSet(BZFILE *compressedFile);
};

class TracerStaticInstRec {
  TracerMemSet readSet;
  TracerMemSet writeSet;
public:
  TracerMemSet &getReadSet();
  TracerMemSet &getWriteSet();
  void dumpRecord(BZFILE *compressedFile);
  void dumpReadSet(BZFILE *compressedFile);
  void dumpWriteSet(BZFILE *compressedFile);
};


class TracerMemoryTrace : public std::map<uintptr_t, TracerStaticInstRec *>{
  string outputDirectory;
public:
  TracerMemoryTrace() : outputDirectory(".") {
    char *env = getenv("LIBCAM_OUTPUT_DIRECTORY");
    if (env) {
      outputDirectory = env;
    }
    // Ensure the memtrace directory exists and is empty
    if(system(("mkdir -p " + outputDirectory + "/memory_accesses").c_str())){ cerr << "mkdir memory_accesses failed\n"; abort(); }
    if(system(("rm -f " + outputDirectory + "/memory_accesses/memory_accesses.*.*.txt.bz2").c_str())) { cerr << "clean memory_accesses failed\n"; abort(); }
  }
  ~TracerMemoryTrace();
  void dumpMemoryTrace(void);
  void clear();
  string getOutputDirectory(){ return outputDirectory; }
};


class MemTraceMemory : public CamMemoryAllocator
{
  JITUINT64 dumpTraceMemUsage;   /**< Memory size trigger for dumping the trace. */

public:
  MemTraceMemory()
    : dumpTraceMemUsage(LIBCAM_DEFAULT_MAX_MEM_USAGE)
  {
    char *env = getenv("LIBCAM_MEM_TRACE_MAX_MEM_USAGE");
    if (env) {
      dumpTraceMemUsage = atoi(env);
    }
  }

  void checkDumpTrace(void);
};


static TracerMemoryTrace *memoryTrace = NULL;
static MemTraceMemory *memoryAllocator = NULL;
static TimeoutCounter *timeoutCounter = NULL;

void
TracerMemSetEntry::init(uintptr_t b, intptr_t s, uint64_t l, uint64_t st, uint64_t e)
{
  base = b; stride = s; length = l; start = st; end = e;
}

uintptr_t TracerMemSetEntry::prediction(){
  return base + stride*getNumInstances();
}

uintptr_t TracerMemSetEntry::getBase() const { return base; }
intptr_t TracerMemSetEntry::getStride() const { return stride; }
uintptr_t TracerMemSetEntry::getLength() const { return length; }
uintptr_t TracerMemSetEntry::getStart() const { return start; }
uintptr_t TracerMemSetEntry::getEnd() const {return end; }
void TracerMemSetEntry::setBase(uintptr_t x) { base = x; }
void TracerMemSetEntry::setStride(intptr_t x) { stride = x; }
void TracerMemSetEntry::setLength(uint64_t x) { length = x; }
void TracerMemSetEntry::setStart(uint64_t x) { start = x; }
void TracerMemSetEntry::setEnd(uint64_t x) { end = x; }

uintptr_t TracerMemSetEntry::getNumInstances(){
  return end - start + 1;
}
void TracerMemSetEntry::incEnd() { end++; }

TracerMemSet::~TracerMemSet(){
  for(TracerMemSet::const_iterator i = begin(); i != end(); i++) {
    memoryAllocator->deleteMem(*i);
  }
  erase(begin(), end());
}

void TracerMemSet::newMemSetEntry(uintptr_t b, intptr_t s, uint64_t l, uint64_t st, uint64_t e){
    TracerMemSetEntry *entry = memoryAllocator->newMem<TracerMemSetEntry>();
    entry->init(b, s, l, st, e);
    push_back(entry);
}

void TracerMemSet::recordMemoryReference(uintptr_t addr, uint64_t len){
  if (size() == 0) {
    newMemSetEntry(addr, 0, len, 0, 0);
  } else {
    TracerMemSetEntry *prev = back();
    if (prev->getLength() != len) {
      /* Data widths do not match, a new pattern must be started */
      newMemSetEntry(addr, 0, len, prev->getEnd() + 1, prev->getEnd() + 1);
    }
    else if(prev->getStart() == prev->getEnd()){
      /* prev is the first entry for this pattern, establish the stride */
      prev->setStride(addr - prev->getBase());
      prev->incEnd();
    }
    else{
      /* A pattern is already established, check if we can match it */
      if(prev->prediction() == addr)
        prev->incEnd();
      else {
        newMemSetEntry(addr, 0, len, prev->getEnd() + 1, prev->getEnd() + 1);
      }
    }
  }
}

TracerMemSet &TracerStaticInstRec::getReadSet() { return readSet; }
TracerMemSet &TracerStaticInstRec::getWriteSet() { return writeSet; }


TracerMemoryTrace::~TracerMemoryTrace()
{
}


void
TracerMemoryTrace::clear(void)
{
  for(TracerMemoryTrace::const_iterator i = begin(); i != end(); i++) {
    memoryAllocator->deleteMem(i->second);
  }
  erase(begin(), end());
}


void
TracerMemSetEntry::dumpInitialEntry(BZFILE *compressedFile)
{
  char buf[DIM_BUF];
  snprintf(buf, DIM_BUF, "%" PRIuPTR " %" PRIdPTR " %" PRIu64 " %" PRIu64 ",", base, stride, length, end - start + 1);
  writeCompressedFile(compressedFile, buf);
}

void
TracerMemSetEntry::dumpEntry(BZFILE *compressedFile, TracerMemSetEntry* prev)
{
  char buf[DIM_BUF];
  snprintf(buf, DIM_BUF, "%" PRIdPTR " %" PRIdPTR " %" PRIu64 " %" PRIu64 ",", (intptr_t)(base) - (intptr_t)(prev->base), stride, length, end - start + 1);
  writeCompressedFile(compressedFile, buf);
}


void
TracerMemSet::dumpSet(BZFILE *compressedFile)
{
  char buf[DIM_BUF];
  snprintf(buf, DIM_BUF, "%zu ", size());
  writeCompressedFile(compressedFile, buf);
  if(!empty()){
    front()->dumpInitialEntry(compressedFile);
    for(const_iterator i = next(begin(), 1); i != end(); i++) {
      (*i)->dumpEntry(compressedFile, *next(i, -1));
    }
  }
  snprintf(buf, DIM_BUF, "\n");
  writeCompressedFile(compressedFile, buf);
}


void
TracerStaticInstRec::dumpRecord(BZFILE *compressedFile)
{
  readSet.dumpSet(compressedFile);
  writeSet.dumpSet(compressedFile);
}

void
TracerStaticInstRec::dumpReadSet(BZFILE *compressedFile)
{
  readSet.dumpSet(compressedFile);
}

void
TracerStaticInstRec::dumpWriteSet(BZFILE *compressedFile)
{
  writeSet.dumpSet(compressedFile);
}

pair<FILE *, BZFILE *> openCompressedStream(string filename){
  /* Open file for appending */
  FILE *f = fopen(filename.c_str(), "a");
  if (!f) {
    perror("Error opening file: ");
    abort();
  }

  /* Prepare to write compressed data */
  int status;
  BZFILE *bzf = BZ2_bzWriteOpen(&status, f, 9, 0, 250);
  if (status != BZ_OK) {
    if(status == BZ_MEM_ERROR)
      cerr << "BZ2: Cannot allocate memory\n";
    abort();
  }
  return pair<FILE *, BZFILE *>(f, bzf);
}

void closeCompressedStream(pair<FILE *, BZFILE *> stream){
  /* Flush and close compressed stream (reduces memory consumption) */
  int status;
  BZ2_bzWriteClose(&status, stream.second, 0, NULL, NULL);
  if(status != BZ_OK){
    cerr << "BZ2: BZ2_bzWriteClose error: " << status << endl;
    abort();
  }
  fclose(stream.first);
}

void
TracerMemoryTrace::dumpMemoryTrace(void)
{
  char buf[DIM_BUF];

  /* Dump the trace. */
  for(TracerMemoryTrace::const_iterator i = begin(); i != end(); i++) {
    
    /* Dump read trace */
    if(i->second->getReadSet().size() > 0){
      pair<FILE *, BZFILE*> stream = openCompressedStream(
        outputDirectory + "/memory_accesses/memory_accesses." + to_string(i->first) + ".r.txt.bz2");
      /* Write compressed data */
      snprintf(buf, DIM_BUF, "%" PRIuPTR "\n", i->first);
      writeCompressedFile(stream.second, buf);
      i->second->dumpReadSet(stream.second);
      snprintf(buf, DIM_BUF, "0\n");
      writeCompressedFile(stream.second, buf);
      closeCompressedStream(stream);
    }
    /* Dump write trace */
    if(i->second->getWriteSet().size() > 0){
      pair<FILE *, BZFILE*> stream = openCompressedStream(
        outputDirectory + "/memory_accesses/memory_accesses." + to_string(i->first) + ".w.txt.bz2");
      /* Write compressed data */
      snprintf(buf, DIM_BUF, "%" PRIuPTR "\n", i->first);
      writeCompressedFile(stream.second, buf);
      snprintf(buf, DIM_BUF, "0\n");
      writeCompressedFile(stream.second, buf);
      i->second->dumpWriteSet(stream.second);
      closeCompressedStream(stream);
    }
  }
}


void
MemTraceMemory::checkDumpTrace(void)
{
  // static JITNINT numDumps = 0;
  if (memUsed > dumpTraceMemUsage) {
    // cerr << "Dumping memory trace " << numDumps << " with allocation of " << memUsed  << endl;
    memoryTrace->dumpMemoryTrace();
    memoryTrace->clear();
    // numDumps += 1;
  }
}

void 
CAM_forceMemTraceDump(){
  memoryTrace->dumpMemoryTrace();
  memoryTrace->clear();
}


/**
 * Record an instruction accessing memory.
 **/
void
CAM_mem(inst_id_t id, uintptr_t raddr1, uint64_t rlen1, uintptr_t raddr2, uint64_t rlen2, uintptr_t waddr, uint64_t wlen)
{
  if(timeoutCounter->recordOperation())
    return;

  TracerStaticInstRec *rec;
  if (memoryTrace->find(id) == memoryTrace->end()) {
    rec = memoryAllocator->newMem<TracerStaticInstRec>();
    (*memoryTrace)[id] = rec;
  } else {
    rec = (*memoryTrace)[id];
  }
  if(rlen1 > 0) {
    rec->getReadSet().recordMemoryReference(raddr1, rlen1);
  }
  if(rlen2 > 0) {
    rec->getReadSet().recordMemoryReference(raddr2, rlen2);
  }
  if(wlen > 0) {
    rec->getWriteSet().recordMemoryReference(waddr, wlen);
  }
  memoryAllocator->checkDumpTrace();
}


/**
 * Initialisation.
 **/
void
memory_trace_init(void)
{
  memoryAllocator = new MemTraceMemory();
  memoryTrace = memoryAllocator->newMem<TracerMemoryTrace>();

  timeoutCounter = new TimeoutCounter();
}


/**
 * Shut down.
 **/
void
memory_trace_shutdown(void)
{
  timeoutCounter->dumpStats(memoryTrace->getOutputDirectory());
  delete timeoutCounter;

  if (memoryTrace) {
    if(memoryTrace->size() > 0) {
      memoryTrace->dumpMemoryTrace();
    } else {
      cerr << "LIBCAM: Memory tracer recorded no instructions\n";
    }
    memoryAllocator->deleteMem(memoryTrace);
    delete memoryAllocator;
  } else {
    cerr << "LIBCAM: Attempt to shut down non-existent memory tracer\n";
  }
}
