
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

#ifndef PARSER_WRAPPERS_H
#define PARSER_WRAPPERS_H

#include "static_inst_rec.h"
#include "StaticLoopRec.h"
#include "CallTrace.h"
#include <set>
using namespace std;

#define DEP_RAW         0x1
#define DEP_WAR         0x2
#define DEP_WAW         0x4
#define DEP_MRAW        0x8
#define DEP_MWAR        0x10
#define DEP_MWAW        0x20

/* Loop trace */
void loopTraceInit();
pair<set<uintptr_t>, uint64_t> parseLoopTraceForInstrList();
int loopTraceGetNextEntry(LoopTraceEntry& loopEntry);

/* Call trace */
void callTraceInit();
StreamParseCallTrace parseCallTrace();
pair<set<uintptr_t>, set<uintptr_t>> parseCallTraceForInstrList();
int callTraceGetNextEntry(CallTraceLoopInvocationGroup& callEntry);

/* Memory trace */
MemoryTrace parse_memory_trace();
MemoryTrace parse_memory_trace_parallel();
pair<vector<uintptr_t>, vector<uintptr_t>> findMemoryTraces();

/* DDG */
pair<set<pair<uintptr_t, uintptr_t>>, set<pair<uintptr_t, uintptr_t>>> parse_dependence_pairs();
vector<tuple<uintptr_t, uintptr_t, unsigned int>> parse_static_ddg();
vector<tuple<uintptr_t, uintptr_t, unsigned int>> parse_ddg(string ddgfile);

#endif
