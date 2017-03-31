
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

#include <assert.h>
#include <xanlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include "cam.h"
#include "MemoryTracer.h"
#include "loop_trace.hh"

using namespace std;

static void segFaultHandler(int nSig)
{
  fprintf(stderr, "Caught segfault: %i: %s\n", errno, strerror(errno));
  exit(50);
}

static void setSegFaultHandler(){
  signal(SIGSEGV, segFaultHandler);
}

void CAM_init (cam_mode_t mode) {
  setSegFaultHandler();
  if (mode == CAM_MEMORY_PROFILE) {
    memory_trace_init();
  } else if (mode == CAM_LOOP_PROFILE) {
    loop_trace_init();
  }
}

void CAM_shutdown (cam_mode_t mode) {
  if (mode == CAM_MEMORY_PROFILE) {
    memory_trace_shutdown();
  } else if (mode == CAM_LOOP_PROFILE) {
    loop_trace_shutdown();
  }
}
